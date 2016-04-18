/* -*- mode: c++ -*-
 *
 * pointing/input/osx/osxPointingDeviceManager.cpp --
 *
 * Initial software
 * Authors: Izzatbek Mukhanov
 * Copyright © Inria
 *
 * http://libpointing.org/
 *
 * This software may be used and distributed according to the terms of
 * the GNU General Public License version 2 or any later version.
 *
 */

#include <pointing/input/osx/osxPointingDeviceManager.h>

#include <pointing/utils/osx/osxPlistUtils.h>
#include <pointing/input/osx/osxHIDUtils.h>

#include <stdexcept>

// FIXME Should I use CFRetain
// seize

namespace pointing {

#define USE_CURRENT_RUNLOOP 0

  void osxPointingDeviceManager::FillDescriptor(IOHIDDeviceRef devRef, PointingDeviceDescriptor &desc)
  {
    desc.devURI = hidDeviceURI(devRef).asString();
    desc.name = hidDeviceName(devRef);
    desc.vendorID = hidDeviceGetIntProperty(devRef, CFSTR(kIOHIDVendorIDKey));
    desc.productID = hidDeviceGetIntProperty(devRef, CFSTR(kIOHIDProductIDKey));
  }

  void osxPointingDeviceManager::convertAnyCandidates()
  {
    for (PointingList::iterator it = candidates.begin(); it != candidates.end(); it++)
    {
      osxPointingDevice *device = *it;
      if (!device->anyURI.asString().empty())
        device->uri = anyToSpecific(device->anyURI);
    }
  }

  void osxPointingDeviceManager::matchCandidates()
  {
    convertAnyCandidates();
    PointingList::iterator i = candidates.begin();
    while (i != candidates.end())
    {
      osxPointingDevice *device = *i;
      bool found = false;
      for(devMap_t::iterator it = devMap.begin(); it != devMap.end(); it++)
      {
        PointingDeviceData *pdd = it->second;
        // Found matching device
        // Move it from candidates to devMap
        if (pdd->desc.devURI == device->uri.asString())
        {
          candidates.erase(i++);
          pdd->pointingList.push_back(device);
          device->devRef = pdd->devRef;
          found = true;
          break;
        }
      }
      if (!found)
        i++;
    }
  }
  /*
  IOHIDDeviceRef osxPointingDeviceManager::findDevRefByURI(const URI &uri)
  {
    for(descMap_t::iterator it = descMap.begin(); it != descMap.end(); it++)
    {
      PointingDeviceData *pdd = it->second;
      if (pdd->desc.devURI == uri.asString())
        return it->first;
    }
    return NULL;
  }
  */
  void osxPointingDeviceManager::AddDevice(void *sender, IOReturn, void *, IOHIDDeviceRef devRef)
  {
    osxPointingDeviceManager *self = (osxPointingDeviceManager *)sender;
    PointingDeviceData *pdd = new PointingDeviceData;
    self->devMap[devRef] = pdd;
    pdd->devRef = devRef;
    FillDescriptor(devRef, pdd->desc);
    self->addDevice(pdd->desc);

    CFDataRef descriptor = (CFDataRef)IOHIDDeviceGetProperty(devRef, CFSTR(kIOHIDReportDescriptorKey));
    if (descriptor) {
      const UInt8 *bytes = CFDataGetBytePtr(descriptor);
      CFIndex length = CFDataGetLength(descriptor);
      if (!pdd->parser.setDescriptor(bytes, length))
        std::cerr << "osxPointingDeviceManager::AddDevice: unable to parse the HID report descriptor" << std::endl;
      else
      {
        IOHIDDeviceRegisterInputReportCallback(devRef, pdd->report, sizeof(pdd->report),
                                               hidReportCallback, self);
      }

      //if (self->debugLevel > 1)
      //{
        std::cerr << "HID descriptors: [ " << std::flush ;
        for (int i=0; i<length; ++i)
          std::cerr << std::hex << std::setfill('0') << std::setw(2) << (int)bytes[i] << " " ;
        std::cerr << "]" << std::endl ;
      //}
    }

    self->matchCandidates();
  }

  void osxPointingDeviceManager::RemoveDevice(void *sender, IOReturn, void *, IOHIDDeviceRef devRef)
  {
    osxPointingDeviceManager *self = (osxPointingDeviceManager *)sender;
    devMap_t::iterator it = self->devMap.find(devRef);
    if (it != self->devMap.end())
    {
      PointingDeviceData *pdd = it->second;
      self->removeDevice(pdd->desc);
      for (PointingList::iterator it = pdd->pointingList.begin(); it != pdd->pointingList.end(); it++)
      {
        osxPointingDevice *device = *it;
        device->devRef = NULL;
        self->candidates.push_back(device);
      }
      delete pdd;
      self->devMap.erase(it);
    }
    self->matchCandidates();
  }

  osxPointingDeviceManager::osxPointingDeviceManager()
  {
    manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone) ;
    if (!manager)
      throw std::runtime_error("IOHIDManagerCreate failed");

    const char *plist = hidDeviceFromVendorProductUsagePageUsage(0, 0, kHIDPage_GenericDesktop, kHIDUsage_GD_Mouse).c_str() ;
    CFMutableDictionaryRef device_match = (CFMutableDictionaryRef)getPropertyListFromXML(plist) ;
    IOHIDManagerSetDeviceMatching(manager, device_match) ;

    IOHIDManagerRegisterDeviceMatchingCallback(manager, AddDevice, (void*)this) ;
    IOHIDManagerRegisterDeviceRemovalCallback(manager, RemoveDevice, (void*)this) ;

#if USE_CURRENT_RUNLOOP
    CFRunLoopRef runLoop = CFRunLoopGetCurrent() ;
    CFStringRef runLoopMode = kCFRunLoopDefaultMode ;
#else
    CFRunLoopRef runLoop = CFRunLoopGetMain() ;
    CFStringRef runLoopMode = kCFRunLoopCommonModes ;
#endif
    IOHIDManagerScheduleWithRunLoop(manager, runLoop, runLoopMode) ;

    if (IOHIDManagerOpen(manager, kIOHIDOptionsTypeNone)!=kIOReturnSuccess)
      throw std::runtime_error("IOHIDManagerOpen failed") ;
  }

  void osxPointingDeviceManager::addPointingDevice(osxPointingDevice *device)
  {
    candidates.push_back(device);
    matchCandidates();
  }

  void osxPointingDeviceManager::removePointingDevice(osxPointingDevice *device)
  {
    URI uri = device->getURI();
    for(devMap_t::iterator it = devMap.begin(); it != devMap.end(); it++)
    {
      PointingDeviceData *pdd = it->second;
      if (pdd->desc.devURI == uri.asString())
      {
        pdd->pointingList.remove(device);
        break;
      }
    }
    candidates.remove(device);
  }

  void osxPointingDeviceManager::hidReportCallback(void *context, IOReturn, void *dev, IOHIDReportType, uint32_t, uint8_t *report, CFIndex)
  {
    TimeStamp::inttime timestamp = TimeStamp::createAsInt();

    osxPointingDeviceManager *self = (osxPointingDeviceManager*)context;
    IOHIDDeviceRef devRef = (IOHIDDeviceRef)dev;

    devMap_t::iterator it = self->devMap.find(devRef);

    if (it != self->devMap.end())
    {
      PointingDeviceData *pdd = it->second;
      if (pdd->parser.setReport(report))
      {
        int dx = 0, dy = 0, buttons = 0;
        pdd->parser.getReportData(&dx, &dy, &buttons) ;
        for (PointingList::iterator pit = pdd->pointingList.begin(); pit != pdd->pointingList.end(); pit++)
        {
          osxPointingDevice *device = *pit;
          device->registerTimestamp(timestamp);
          if (device->callback)
            device->callback(device->callback_context, timestamp, dx, dy, buttons);
        }
      }
    }
  }
}
