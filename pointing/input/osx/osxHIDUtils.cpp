/*
 *
 * pointing/input/osx/osxHIDUtils.cpp --
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

#include <pointing/utils/TimeStamp.h>
#include <pointing/input/osx/osxHIDUtils.h>

#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/hid/IOHIDElement.h>
#include <IOKit/hid/IOHIDUsageTables.h>
#include <IOKit/hidsystem/IOHIDLib.h>

#include <stdexcept>
#include <sstream>

#include <mach/mach_time.h>

namespace pointing {

  // -----------------------------------------------------------------------

  static std::string
  CFStringToSTLString(const CFStringRef cfstr) {
    if (!cfstr) return "" ;

    CFIndex size = CFStringGetMaximumSizeForEncoding(CFStringGetLength(cfstr), 
						     kCFStringEncodingUTF8) ;
    char *buffer = new char[size] ;
    if (!CFStringGetCString(cfstr, buffer, size, kCFStringEncodingUTF8))
      throw std::runtime_error("CFStringGetCString failed") ;

    std::string result(buffer) ;
    delete [] buffer ;
    return result ;
  }

  int32_t
  hidDeviceGetIntProperty(IOHIDDeviceRef device, CFStringRef prop, int32_t defval) {
    int32_t result = defval ;
    CFNumberRef n = (CFNumberRef)IOHIDDeviceGetProperty(device, prop) ;
    if (n) CFNumberGetValue(n, kCFNumberSInt32Type, &result) ;
    return result ;
  }

  std::string
  hidDeviceGetStringProperty(IOHIDDeviceRef device, CFStringRef prop, std::string defval) {
    std::string result = defval ;
    CFStringRef s = (CFStringRef)IOHIDDeviceGetProperty(device, prop) ;
    if (s) result = CFStringToSTLString(s) ;
    return result ;
  }

  // -----------------------------------------------------------------------

  URI
  hidDeviceURI(IOHIDDeviceRef device) {
    io_name_t className ;
    IOObjectGetClass(IOHIDDeviceGetService(device), className) ;
    std::stringstream uriAsString ;
    uriAsString << "osxhid:/"
		<< hidDeviceGetStringProperty(device, CFSTR(kIOHIDTransportKey))
		<< "/"
		<< std::hex << hidDeviceGetIntProperty(device, CFSTR(kIOHIDLocationIDKey)) << std::dec
		<< "/" << className ;
    return URI(uriAsString.str()) ;
  }

  // -----------------------------------------------------------------------

  std::string hidDeviceName(IOHIDDeviceRef device)
  {
      std::string manufacturer = hidDeviceGetStringProperty(device, CFSTR(kIOHIDManufacturerKey)) ;
      std::string product = hidDeviceGetStringProperty(device, CFSTR(kIOHIDProductKey)) ;
      return manufacturer + " - " + product;
  }

  // -----------------------------------------------------------------------

  void
  hidDebugDevice(IOHIDDeviceRef device, std::ostream& out) {
    int32_t vendorID = hidDeviceGetIntProperty(device, CFSTR(kIOHIDVendorIDKey)) ;
    int32_t productID = hidDeviceGetIntProperty(device, CFSTR(kIOHIDProductIDKey)) ;
    std::string manufacturer = hidDeviceGetStringProperty(device, CFSTR(kIOHIDManufacturerKey)) ;
    std::string product = hidDeviceGetStringProperty(device, CFSTR(kIOHIDProductKey)) ;

    out << hidDeviceURI(device).asString()
	<< " ["
	<< std::hex
	<< "vend:0x" << vendorID << ", prod:0x" << productID
	<< std::dec ;
    SInt32 resolution = hidGetPointingResolution(device) ;
    if (resolution!=-1) out << ", " << resolution << " CPI" ;
    double reportinterval = hidGetReportInterval(device) ;
    if (reportinterval!=-1) out << ", " << 1.0/reportinterval << " Hz" ;
    out << " - "
	<< product
	<< " (" << manufacturer << ")" 
	<< "]" ;

    // std::string tmp = hidDeviceGetStringProperty(device, CFSTR(kIOHIDPointerAccelerationTableKey)) ;
    // out << tmp << std::endl ;
  }

  uint64_t
  AbsoluteTimeInNanoseconds(uint64_t tAbs) {
    // If this is the first time we've run, get the timebase.  We can
    // use denom == 0 to indicate that sTimebaseInfo is uninitialised
    // because it makes no sense to have a zero denominator is a
    // fraction.

    static mach_timebase_info_data_t sTimebaseInfo ;
    if ( sTimebaseInfo.denom == 0 )
      (void) mach_timebase_info(&sTimebaseInfo) ;

    // Do the maths. We hope that the multiplication doesn't overflow;
    // the price you pay for working in fixed point.

    uint64_t tNano = tAbs * sTimebaseInfo.numer / sTimebaseInfo.denom ;
    return tNano ;
  }

  void
  hidDebugValue(IOHIDValueRef hidvalue, std::ostream& out) {
    TimeStamp::inttime timestamp = AbsoluteTimeInNanoseconds(IOHIDValueGetTimeStamp(hidvalue)) ;

    CFIndex value = IOHIDValueGetIntegerValue(hidvalue) ;
    IOHIDElementRef element = IOHIDValueGetElement(hidvalue) ;
    uint32_t usagepage = IOHIDElementGetUsagePage(element) ;
    uint32_t usage = IOHIDElementGetUsage(element) ;
    out 
      << "@abs" << timestamp << "ns"
      << std::hex << " 0x" << usagepage << "/0x" << usage << std::dec
      << ": " ;
    if (usagepage==kHIDPage_GenericDesktop) {
      switch (usage) {
      case kHIDUsage_GD_X: out << "GD_X" ; break ;
      case kHIDUsage_GD_Y: out << "GD_Y" ; break ;
      case kHIDUsage_GD_Z: out << "GD_Z" ; break ;
      case kHIDUsage_GD_Wheel: out << "GD_Wheel" ; break ;
      default: out << "GD_???" ; break ;
      }
      out << " " << value ;
    } else if (usagepage==kHIDPage_Digitizer) {
      switch (usage) {
      case kHIDUsage_Dig_TipPressure: out << "Dig_TipPressure" ; break ;
      case kHIDUsage_Dig_InRange: out << "Dig_InRange" ; break ;
      case kHIDUsage_Dig_TipSwitch: out << "Dig_TipSwitch" ; break ;
      case 0x47: out << "Dig_MSWin_Confidence" ; break ;
      case 0x48: out << "Dig_MSWin_Width" ; break ;
      case 0x49: out << "Dig_MSWin_Height" ; break ;
      case 0x51: out << "Dig_MSWin_ContactID" ; break ;
      case 0x52: out << "Dig_MSWin_InputMode" ; break ;
      case 0x53: out << "Dig_MSWin_DeviceIndex" ; break ;
      case 0x54: out << "Dig_MSWin_ContactCount" ; break ;
      case 0x55: out << "Dig_MSWin_ContactMax" ; break ;
      default: out << "Dig_???" ; break ;
      }
      out << " " << value ;
    } else if (usagepage==kHIDPage_Button) {
      out << "Btn_" << usage << " " << value ;
    } else if (usagepage==kHIDPage_KeyboardOrKeypad) {
      out << "Key_" << usage << " " << value ;
    } else
      out << "??? " << value ;
    out << " (phy="
	<< IOHIDValueGetScaledValue(hidvalue, kIOHIDValueScaleTypePhysical)
	<< ", cal="
	<< IOHIDValueGetScaledValue(hidvalue, kIOHIDValueScaleTypeCalibrated)
	<< ")"
	<< std::endl ;
  }

  // -----------------------------------------------------------------------

  io_service_t
  hidGetParentService(IOHIDDeviceRef device, io_name_t classname) {
    return hidGetParentService(IOHIDDeviceGetService(device), classname) ;
  }

  io_service_t
  hidGetParentService(io_service_t service, io_name_t classname) {
    for (io_service_t result=service; result!=MACH_PORT_NULL;) {
#if 0
      io_name_t name ;
      IOObjectGetClass(result, name) ;
      std::cerr << "service: " << result << ", className: " << name << " (" << className << ")" << std::endl ;
#endif
      if (IOObjectConformsTo(result, classname)) {
	if (result==service) IOObjectRetain(result) ;
	return result ;
      }
      io_service_t parent = MACH_PORT_NULL ;
      // IORegistryEntryGetParentEntry retains the returned object
      if (IORegistryEntryGetParentEntry(result, kIOServicePlane, &parent)!=KERN_SUCCESS)
	parent = MACH_PORT_NULL ;
      if (result!=service) IOObjectRelease(result) ;
      result = parent ;
    }
    return MACH_PORT_NULL ;
  }

  // -----------------------------------------------------------------------

  SInt32
  hidGetPointingResolution(IOHIDDeviceRef device) {
    if (!device) return -1 ;
    return hidGetPointingResolution(IOHIDDeviceGetService(device)) ;
  }

  SInt32
  hidGetPointingResolution(io_service_t service) {
    if (service==MACH_PORT_NULL) return -1 ;

    SInt32 resolution = -1 ;

    if (IOObjectConformsTo(service, "IOHIDPointing")) {
      CFTypeRef res = IORegistryEntryCreateCFProperty(service, CFSTR(kIOHIDPointerResolutionKey), kCFAllocatorDefault, kNilOptions) ;
      if (res) {
	if (CFGetTypeID(res)==CFNumberGetTypeID() 
	    && CFNumberGetValue((CFNumberRef)res, kCFNumberSInt32Type, &resolution))
	  resolution = resolution>>16 ;
	CFRelease(res) ;
      }
      return resolution ;
    }

#if 1
    io_service_t pointing = hidGetParentService(service, (char*)"IOHIDPointing") ;
    if (pointing!=MACH_PORT_NULL) {
      resolution = hidGetPointingResolution(pointing) ;
      IOObjectRelease(pointing) ;
      return resolution ;
    }
#endif

#if 0  
    io_iterator_t children ;
    IORegistryEntryGetChildIterator(service, kIOServicePlane, &children) ;
    io_object_t child ;
    while (resolution==-1 && (child = IOIteratorNext(children))) {
      resolution = hidGetPointingResolution(child) ;
      IOObjectRelease(child) ;
    }
    IOObjectRelease(children) ;
#endif

    return resolution ;
  }

  // -----------------------------------------------------------------------

  double
  hidGetReportInterval(IOHIDDeviceRef device) {
    int32_t microseconds = hidDeviceGetIntProperty(device, CFSTR(kIOHIDReportIntervalKey), -1) ;
    return microseconds<0 ? -1.0 : microseconds/1000000.0 ;
  }

  // -----------------------------------------------------------------------

  IOUSBInterfaceInterface190 **
  getUSBInterface(io_service_t hiddriver, 
		  int usbclass, int usbsubclass, int endpoints) {
    IOUSBInterfaceInterface190 **usbinterface = 0 ;

    // IOUSBDevice --> IOUSBInterface --> IOUSBHIDDriver
    io_service_t hidinterface, usbdevice ;
    IORegistryEntryGetParentEntry(hiddriver, kIOServicePlane, &hidinterface) ;
    IORegistryEntryGetParentEntry(hidinterface, kIOServicePlane, &usbdevice) ;

    io_iterator_t children ;
    IORegistryEntryGetChildIterator(usbdevice, kIOServicePlane, &children) ;
    io_object_t child ;
    while ((child = IOIteratorNext(children))) {
      if (usbinterface==0
	  && IOObjectConformsTo(child, "IOUSBInterface") 
	  && !IOObjectIsEqualTo(child, hidinterface)) {
	IOCFPlugInInterface **plugInInterface = 0 ;
	SInt32 score;
	int kr = IOCreatePlugInInterfaceForService(child,
						   kIOUSBInterfaceUserClientTypeID,
						   kIOCFPlugInInterfaceID,
						   &plugInInterface, &score) ;
	if (kr==0 && plugInInterface) {
	  IOUSBInterfaceInterface190 **interface = 0 ;
	  kr = (*plugInInterface)->QueryInterface(plugInInterface,
						  CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID190),
						  (LPVOID *) &interface) ;
	  (*plugInInterface)->Release(plugInInterface) ;
	  if (kr==0 && interface) {
	    UInt8 ifClass=0, ifSubClass=0, ifEndpoints=0 ;
	    (*interface)->GetInterfaceClass(interface, &ifClass) ;
	    (*interface)->GetInterfaceSubClass(interface, &ifSubClass) ;
	    (*interface)->GetNumEndpoints(interface, &ifEndpoints) ;
	    if (ifClass==usbclass
		&& ifSubClass==usbsubclass
		&& ifEndpoints==endpoints) {
	      // std::cerr << "Youpee: " << interface << std::endl ;
	      usbinterface = interface ;
	    } else
	      (void)(*interface)->Release(interface) ;
	  }
	}
      }
      IOObjectRelease(child) ;
    }
    IOObjectRelease(children) ;
    IOObjectRelease(hidinterface) ;
    IOObjectRelease(usbdevice) ;

    if (usbinterface) {
      if ((*usbinterface)->USBInterfaceOpenSeize(usbinterface)) {
	(void) (*usbinterface)->Release(usbinterface) ;
	usbinterface = 0 ;
      }
    }

    return usbinterface ;
  }

  // -----------------------------------------------------------------------

  std::string hidDeviceFromVendorProductUsagePageUsage(int vendorID, int productID, 
						       int primaryUsagePage, int primaryUsage) {
#if 0
    std::cerr << "hidDeviceFromVendorProductUsagePageUsage: " 
	      << "vid: " << vendorID
	      << ", pid: " << productID
	      << ", page: " << primaryUsagePage
	      << ", usage: " << primaryUsage
	      << std::endl ;
#endif

    std::stringstream result ;
    result << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
	   << "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">"
	   << "<plist version=\"1.0\">"
	   << "<dict>" ;
    if (vendorID)
      result << "<key>VendorID</key><integer>" << vendorID << "</integer>" ;
    if (productID)
      result << "<key>ProductID</key><integer>" << productID << "</integer>" ;
    if (primaryUsagePage)
      result << "<key>PrimaryUsagePage</key><integer>" << primaryUsagePage << "</integer>" ;
    if (primaryUsage)
      result << "<key>PrimaryUsage</key><integer>" << primaryUsage << "</integer>" ;
    result << "</dict>"
	   << "</plist>" ;
    return result.str() ;
  }

  std::string hidAnyPointingDevice(void) {
#if 1
    return hidDeviceFromVendorProductUsagePageUsage(0,0,kHIDPage_GenericDesktop,kHIDUsage_GD_Mouse) ;
#else
    static const char *anypointingdevice = "<?xml version='1.0' encoding='UTF-8'?><!DOCTYPE plist PUBLIC '-//Apple//DTD PLIST 1.0//EN' 'http://www.apple.com/DTDs/PropertyList-1.0.dtd'><plist version='1.0'><dict><key>PrimaryUsagePage</key><integer>1</integer><key>PrimaryUsage</key><integer>2</integer></dict></plist>" ;
    return anypointingdevice ; 
#endif
  }

  std::string hidXYElements(void) { 
    static const char *xyelements = "<?xml version='1.0' encoding='UTF-8'?><!DOCTYPE plist PUBLIC '-//Apple//DTD PLIST 1.0//EN' 'http://www.apple.com/DTDs/PropertyList-1.0.dtd'><plist version='1.0'><array><dict><key>UsagePage</key><integer>1</integer><key>Usage</key><integer>48</integer></dict><dict><key>UsagePage</key><integer>1</integer><key>Usage</key><integer>49</integer></dict></array></plist>" ;
    return xyelements ;
  }

  // -----------------------------------------------------------------------

}
