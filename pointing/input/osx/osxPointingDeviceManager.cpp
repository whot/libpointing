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

namespace pointing {

#define USE_CURRENT_RUNLOOP 0
#define MAX(X, Y)           (((X) > (Y)) ? (X) : (Y))

  void fillDescriptorInfo(IOHIDDeviceRef devRef, PointingDeviceDescriptor &desc)
  {
    desc.devURI = hidDeviceURI(devRef);
    desc.vendor = hidDeviceGetStringProperty(devRef, CFSTR(kIOHIDManufacturerKey));
    desc.product = hidDeviceGetStringProperty(devRef, CFSTR(kIOHIDProductKey));
    desc.vendorID = hidDeviceGetIntProperty(devRef, CFSTR(kIOHIDVendorIDKey));
    desc.productID = hidDeviceGetIntProperty(devRef, CFSTR(kIOHIDProductIDKey));
  }

  void printDevice(IOHIDDeviceRef devRef, bool exists)
  {
    std::cerr << (exists ? "+ " : "  ");
    hidDebugDevice(devRef, std::cerr);
    std::cerr << std::endl;
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
    for(devMap_t::iterator it = devMap.begin(); it != devMap.end(); it++)
    {
      PointingDeviceData *pdd = it->second;

      PointingList::iterator i = candidates.begin();
      while (i != candidates.end())
      {
        osxPointingDevice *device = *i;
        // Found matching device
        // Move it from candidates to devMap
        if (pdd->desc.devURI == device->uri)
        {
          candidates.erase(i++);
          processMatching(pdd, device);
        }
        else
          i++;
      }
    }
  }

  void osxPointingDeviceManager::processMatching(PointingDeviceData *pdd, osxPointingDevice *device)
  {
    pdd->pointingList.push_back(device);
    device->devRef = pdd->devRef;
    // FIXME Maybe look all the candidates for seize option
    // since only the first matching is used to establish the connection
    IOOptionBits inOptions = device->seize ? kIOHIDOptionsTypeSeizeDevice : kIOHIDOptionsTypeNone;
    if (IOHIDDeviceOpen(device->devRef, inOptions) != kIOReturnSuccess)
      throw std::runtime_error("IOHIDDeviceOpen failed");
  }

  void osxPointingDeviceManager::AddDevice(void *sender, IOReturn, void *, IOHIDDeviceRef devRef)
  {
    osxPointingDeviceManager *self = (osxPointingDeviceManager *)sender;
    PointingDeviceData *pdd = new PointingDeviceData;
    self->devMap[devRef] = pdd;
    pdd->devRef = devRef;
    fillDescriptorInfo(devRef, pdd->desc);
    self->addDevice(pdd->desc);
    self->matchCandidates();

    if (self->debugLevel > 0)
      printDevice(devRef, pdd->pointingList.size());

    CFDataRef descriptor = (CFDataRef)IOHIDDeviceGetProperty(devRef, CFSTR(kIOHIDReportDescriptorKey));
    if (descriptor) {
      const UInt8 *bytes = CFDataGetBytePtr(descriptor);
      CFIndex length = CFDataGetLength(descriptor);
      if (!pdd->parser.setDescriptor(bytes, length))
        std::cerr << "osxPointingDeviceManager::AddDevice: unable to parse the HID report descriptor" << std::endl;
      else
        IOHIDDeviceRegisterInputReportCallback(devRef, pdd->report, sizeof(pdd->report),
                                               hidReportCallback, self);

      if (self->debugLevel > 1)
      {
        std::cerr << "HID descriptors: [ " << std::flush ;
        for (int i=0; i<length; ++i)
          std::cerr << std::hex << std::setfill('0') << std::setw(2) << (int)bytes[i] << " " ;
        std::cerr << "]" << std::endl ;
      }
    }
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
        IOHIDDeviceClose(devRef, kIOHIDOptionsTypeNone);
        self->candidates.push_back(device);
      }
      delete pdd;
      self->devMap.erase(it);
    }
    self->matchCandidates();
  }

  osxPointingDeviceManager::osxPointingDeviceManager()
    :debugLevel(0)
  {
    manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
    if (!manager)
      throw std::runtime_error("IOHIDManagerCreate failed");

    const char *plist = hidDeviceFromVendorProductUsagePageUsage(0, 0, kHIDPage_GenericDesktop, kHIDUsage_GD_Mouse).c_str();
    CFMutableDictionaryRef device_match = (CFMutableDictionaryRef)getPropertyListFromXML(plist);
    IOHIDManagerSetDeviceMatching(manager, device_match);

    IOHIDManagerRegisterDeviceMatchingCallback(manager, AddDevice, (void*)this);
    IOHIDManagerRegisterDeviceRemovalCallback(manager, RemoveDevice, (void*)this);

#if USE_CURRENT_RUNLOOP
    CFRunLoopRef runLoop = CFRunLoopGetCurrent();
    CFStringRef runLoopMode = kCFRunLoopDefaultMode;
#else
    CFRunLoopRef runLoop = CFRunLoopGetMain();
    CFStringRef runLoopMode = kCFRunLoopCommonModes;
#endif
    IOHIDManagerScheduleWithRunLoop(manager, runLoop, runLoopMode);
  }

  void osxPointingDeviceManager::addPointingDevice(osxPointingDevice *device)
  {
    debugLevel = MAX(debugLevel, device->debugLevel);
    candidates.push_back(device);
    matchCandidates();
  }

  void osxPointingDeviceManager::removePointingDevice(osxPointingDevice *device)
  {
    URI uri = device->uri;
    for(devMap_t::iterator it = devMap.begin(); it != devMap.end(); it++)
    {
      PointingDeviceData *pdd = it->second;
      if (pdd->desc.devURI == uri)
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
