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

#define OSX_DEFAULT_VENDOR    0
#define OSX_DEFAULT_PRODUCT   0
#define OSX_DEFAULT_USAGEPAGE kHIDPage_GenericDesktop // Find any PointingDevice
#define OSX_DEFAULT_USAGE     kHIDUsage_GD_Mouse      // Find any PointingDevice

namespace pointing {

    void osxPointingDeviceManager::ConvertDevice(IOHIDDeviceRef inDevice, PointingDeviceDescriptor &desc)
    {
        desc.devURI = hidDeviceURI(inDevice).asString();
        desc.name = hidDeviceName(inDevice);
        desc.vendorID = hidDeviceGetIntProperty(inDevice, CFSTR(kIOHIDVendorIDKey));
        desc.productID = hidDeviceGetIntProperty(inDevice, CFSTR(kIOHIDProductIDKey));
    }

    void osxPointingDeviceManager::AddDevice(void *sender, IOReturn, void *, IOHIDDeviceRef device)
    {
        osxPointingDeviceManager *self = (osxPointingDeviceManager *)sender;
        PointingDeviceDescriptor desc;
        ConvertDevice(device, desc);
        self->descMap[device] = desc;
        self->addDevice(desc);
    }

    void osxPointingDeviceManager::RemoveDevice(void *sender, IOReturn, void *, IOHIDDeviceRef device)
    {
        osxPointingDeviceManager *self = (osxPointingDeviceManager *)sender;
        descMap_t::iterator it = self->descMap.find(device);
        if (it != self->descMap.end())
        {
            PointingDeviceDescriptor desc = it->second;
            self->descMap.erase(it);
            self->removeDevice(desc);
        }
    }

    osxPointingDeviceManager::osxPointingDeviceManager()
    {
        manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone) ;
        if (!manager)
            throw std::runtime_error("IOHIDManagerCreate failed") ;

        int vendorID = OSX_DEFAULT_VENDOR ;
        int productID = OSX_DEFAULT_PRODUCT ;
        int primaryUsagePage = OSX_DEFAULT_USAGEPAGE ;
        int primaryUsage = OSX_DEFAULT_USAGE ;

        const char *plist = hidDeviceFromVendorProductUsagePageUsage(vendorID, productID,
                                                            primaryUsagePage, primaryUsage).c_str() ;
        CFMutableDictionaryRef device_match = (CFMutableDictionaryRef)getPropertyListFromXML(plist) ;
        IOHIDManagerSetDeviceMatching(manager, device_match) ;

        IOHIDManagerRegisterDeviceMatchingCallback(manager, AddDevice, this);
        IOHIDManagerRegisterDeviceRemovalCallback(manager, RemoveDevice, this) ;
        IOHIDManagerScheduleWithRunLoop(manager, CFRunLoopGetMain(), kCFRunLoopCommonModes) ;

        IOOptionBits inOptions = kIOHIDOptionsTypeNone ;
        if (IOHIDManagerOpen(manager, inOptions)!=kIOReturnSuccess)
            throw std::runtime_error("IOHIDManagerOpen failed") ;
    }
}
