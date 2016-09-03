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

#if 0
  bool isNotPointingDevice(IOHIDDeviceRef devRef)
  {
    // List of the URI substrings for which the corresponding devices will be ignored
    io_name_t ignored[] = {
      "Keyboard",
      "AppleUSBTCButtons",
      "BNBTrackpadDevice",
      "AppleMikeyHIDDriver",
      "AppleUSBMultitouchDriver"
    } ;
    io_name_t className;
    IOObjectGetClass(IOHIDDeviceGetService(devRef), className);
    const int n = sizeof(ignored) / sizeof(ignored[0]);
    for (int i = 0; i < n; i++)
      if (strstr(className, ignored[i]) != NULL)
        return true;
    return false;
  }
#endif
  
  void fillDescriptorInfo(IOHIDDeviceRef devRef, PointingDeviceDescriptor &desc)
  {
    desc.devURI = hidDeviceURI(devRef);
    desc.vendor = hidDeviceGetStringProperty(devRef, CFSTR(kIOHIDManufacturerKey));
    desc.product = hidDeviceGetStringProperty(devRef, CFSTR(kIOHIDProductKey));
    desc.vendorID = hidDeviceGetIntProperty(devRef, CFSTR(kIOHIDVendorIDKey));
    desc.productID = hidDeviceGetIntProperty(devRef, CFSTR(kIOHIDProductIDKey));
  }

  void osxPointingDeviceManager::processMatching(PointingDeviceData *data, SystemPointingDevice *device)
  {
    osxPointingDevice *dev = static_cast<osxPointingDevice *>(device);
    osxPointingDeviceData *pdd = static_cast<osxPointingDeviceData *>(data);
    IOOptionBits inOptions = dev->seize ? kIOHIDOptionsTypeSeizeDevice : kIOHIDOptionsTypeNone;
    dev->cpi = hidGetPointingResolution(pdd->devRef);
    dev->hz = 1.0 / hidGetReportInterval(pdd->devRef);
    if (IOHIDDeviceOpen(pdd->devRef, inOptions) != kIOReturnSuccess)
    {
      std::cerr << "IOHIDDeviceOpen failed" << std::endl;
      if (inOptions == kIOHIDOptionsTypeSeizeDevice)
        std::cerr << "Could not seize " << device->getURI() << std::endl;
    }
  }

  void osxPointingDeviceManager::AddDevice(void *sender, IOReturn, void *, IOHIDDeviceRef devRef)
  {
    // if (isNotPointingDevice(devRef)) return ; // Prevents other HID devices from being detected
    // NR: No. These devices matched the requested profile, leave that decision to the application    
    
    osxPointingDeviceManager *self = (osxPointingDeviceManager *)sender;
    osxPointingDeviceData *pdd = new osxPointingDeviceData;
    fillDescriptorInfo(devRef, pdd->desc);
    pdd->devRef = devRef;
    self->registerDevice(devRef, pdd);

    CFDataRef descriptor = (CFDataRef)IOHIDDeviceGetProperty(devRef, CFSTR(kIOHIDReportDescriptorKey));
    if (descriptor) {
      const UInt8 *bytes = CFDataGetBytePtr(descriptor);
      CFIndex length = CFDataGetLength(descriptor);
      if (!pdd->parser.setDescriptor(bytes, length))
      {
        if (self->debugLevel > 1)
          std::cerr << "    osxPointingDeviceManager::AddDevice: unable to parse the HID report descriptor" << std::endl;
      }
      else {
	IOHIDDeviceRegisterInputReportCallback(devRef, pdd->report, sizeof(pdd->report), hidReportCallback, self) ;
      }

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
    if (self->unregisterDevice(devRef))
    {
      IOHIDDeviceClose(devRef, kIOHIDOptionsTypeNone);
    }
  }

  osxPointingDeviceManager::osxPointingDeviceManager()
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

    if (IOHIDManagerOpen(manager, kIOHIDOptionsTypeNone)!=kIOReturnSuccess)
      throw std::runtime_error("IOHIDManagerOpen failed");
  }

  osxPointingDeviceManager::~osxPointingDeviceManager()
  {
    if (manager)
    {
      IOHIDManagerClose(manager, kIOHIDOptionsTypeNone) ;
      CFRelease(manager) ;
    }
  }

  void osxPointingDeviceManager::hidReportCallback(void *context, IOReturn, void *dev, IOHIDReportType, uint32_t, uint8_t *report, CFIndex)
  {
    std::cerr << "hidReportCallback" << std::endl ;
    TimeStamp::inttime timestamp = TimeStamp::createAsInt();

    osxPointingDeviceManager *self = static_cast<osxPointingDeviceManager *>(context);
    IOHIDDeviceRef devRef = (IOHIDDeviceRef)dev;

    auto it = self->devMap.find(devRef);

    if (it != self->devMap.end())
    {
      osxPointingDeviceData *pdd = static_cast<osxPointingDeviceData *>(it->second);
      if (pdd->parser.setReport(report))
      {
        int dx = 0, dy = 0, buttons = 0;
        pdd->parser.getReportData(&dx, &dy, &buttons) ;
        for (SystemPointingDevice *device : pdd->pointingList)
        {
          osxPointingDevice *dev = static_cast<osxPointingDevice *>(device);
          dev->registerTimestamp(timestamp, dx, dy);
          if (dev->callback)
            dev->callback(dev->callback_context, timestamp, dx, dy, buttons);
        }
      }
    }
  }
}
