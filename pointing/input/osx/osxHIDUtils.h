/* -*- mode: c++ -*-
 *
 * pointing/input/osx/osxHIDUtils.h --
 *
 * Initial software
 * Authors: Nicolas Roussel
 * Copyright © Inria
 *
 * http://libpointing.org/
 *
 * This software may be used and distributed according to the terms of
 * the GNU General Public License version 2 or any later version.
 *
 */

#ifndef osxHIDUtils_h
#define osxHIDUtils_h

#include <pointing/utils/URI.h>

#include <IOKit/hid/IOHIDDevice.h>
#include <IOKit/hid/IOHIDValue.h>
#include <IOKit/usb/IOUSBLib.h>

#include <iostream>
#include <string>

namespace pointing {

  int32_t hidDeviceGetIntProperty(IOHIDDeviceRef device, CFStringRef prop, int32_t defval=0) ;

  std::string hidDeviceGetStringProperty(IOHIDDeviceRef device, CFStringRef prop, std::string defval="") ;

  URI hidDeviceURI(IOHIDDeviceRef device) ;

  std::string hidDeviceName(IOHIDDeviceRef device);

  void hidDebugDevice(IOHIDDeviceRef device, std::ostream& out) ;

  void hidDebugValue(IOHIDValueRef hidvalue, std::ostream& out) ;

  io_service_t hidGetParentService(IOHIDDeviceRef device, io_name_t classname) ;
  io_service_t hidGetParentService(io_service_t service, io_name_t classname) ;

  // Return -1 if resolution is unknown
  SInt32 hidGetPointingResolution(IOHIDDeviceRef device) ;
  SInt32 hidGetPointingResolution(io_service_t service) ;

  // Result is in seconds (-1 if interval is unknown)
  double hidGetReportInterval(IOHIDDeviceRef device) ;

  IOUSBInterfaceInterface190 **getUSBInterface(io_service_t hiddriver, int usbclass, int usbsubclass, int endpoints) ;

  std::string hidDeviceFromVendorProductUsagePageUsage(int vendorID, int productID,
						       int primaryUsagePage, int primaryUsage) ;
  std::string hidAnyPointingDevice(void) ;
  std::string hidXYElements(void) ;

  uint64_t AbsoluteTimeInNanoseconds(uint64_t tAbs) ;

}

#endif
