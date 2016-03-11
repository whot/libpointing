/* -*- mode: c++ -*-
 *
 * pointing/input/osx/osxHIDInputDevice.cpp --
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

#include <pointing/utils/osx/osxPlistUtils.h>
#include <pointing/input/osx/osxHIDInputDevice.h>
#include <pointing/input/osx/osxHIDUtils.h>

#include <stdexcept>
#include <sstream>
#include <iomanip>

namespace pointing {

  int osxHIDInputDevice::queueSize = 4096 ;

#define OSX_DEFAULT_DEBUGLEVEL 0
#define OSX_DEFAULT_SEIZEDEVICE false

#define USE_CURRENT_RUNLOOP 0

  // -----------------------------------------------------------------------

  osxHIDInputDevice::__device::__device(IOHIDDeviceRef dev) {
    device = dev ;
    CFRetain(device) ;
    queue = 0 ;
  }

  osxHIDInputDevice::__device::~__device() {
    if (queue) {
      // std::cerr << "osxHIDInputDevice(" << this << "): stopping queue" << std::endl ;
      IOHIDQueueStop(queue) ;
      // std::cerr << "osxHIDInputDevice(" << this << "): releasing queue" << std::endl ;
      CFRelease(queue) ;
    }
    // std::cerr << "osxHIDInputDevice(" << this << "): releasing device" << std::endl ;
    CFRelease(device) ;
  }

  // -----------------------------------------------------------------------

  void
  osxHIDInputDevice::AddDevice(void *context, IOReturn /*result*/, void */*sender*/, IOHIDDeviceRef device) {
    osxHIDInputDevice *self = (osxHIDInputDevice*)context ;

    URI devUri = hidDeviceURI(device) ;
    // std::cerr << "osxHIDInputDevice::AddDevice: " << devUri.asString() << std::endl ;
    
    bool match = self->theDevice==0 && (self->uri.isEmpty() || self->uri.scheme=="any" || self->uri.resemble(devUri)) ;

    if (self->debugLevel>0) {
      std::cerr << (match?"+ ":"  ") ; // << device << " " ;
      hidDebugDevice(device, std::cerr) ;
      std::cerr << std::endl ;
    }

    if (!match) return ;

    self->theDevice = new __device(device) ;
    self->uri = devUri ;

    CFDataRef descriptor = (CFDataRef)IOHIDDeviceGetProperty(self->theDevice->device, CFSTR(kIOHIDReportDescriptorKey)) ;
    if (descriptor) {
      const UInt8 *bytes = CFDataGetBytePtr(descriptor) ;
      CFIndex length = CFDataGetLength(descriptor) ;
      if (!self->parser->setDescriptor(bytes, length))
        std::cerr << "osxHIDInputDevice::AddDevice: unable to parse the HID report descriptor" << std::endl;
      if (self->debugLevel > 1)
      {
        std::cerr << "HID descriptors: [ " << std::flush ;
        for (int i=0; i<length; ++i)
          std::cerr << std::hex << std::setfill('0') << std::setw(2) << (int)bytes[i] << " " ;
        std::cerr << "]" << std::endl ;
      }
    }
    
    if (self->inputreport_callback) {
      IOHIDDeviceRegisterInputReportCallback(device, self->theDevice->report, sizeof(self->theDevice->report),
					     self->inputreport_callback, self->inputreport_context) ;
    }

    if (self->value_callback) {
      IOHIDDeviceSetInputValueMatchingMultiple(device, self->elements_match) ; 
      IOHIDDeviceRegisterInputValueCallback(device, self->value_callback, self->value_context) ;
    }

    if (self->queue_callback) {
      self->theDevice->queue = IOHIDQueueCreate(kCFAllocatorDefault, device, queueSize, kIOHIDOptionsTypeNone) ;
      if (self->elements_match) {
	CFIndex mcount = CFArrayGetCount(self->elements_match) ;
	for (CFIndex mindex=0; mindex<mcount; ++mindex) {
	  CFDictionaryRef matching = (CFDictionaryRef)CFArrayGetValueAtIndex(self->elements_match, mindex) ;
	  CFArrayRef elements = IOHIDDeviceCopyMatchingElements(device, matching, kIOHIDOptionsTypeNone) ;
	  if (!elements) continue ;
	  CFIndex ecount = CFArrayGetCount(elements) ;
	  for (CFIndex eindex=0; eindex<ecount; ++eindex) {
	    IOHIDElementRef e = (IOHIDElementRef)CFArrayGetValueAtIndex(elements, eindex) ;
	    IOHIDQueueAddElement(self->theDevice->queue, e) ;
#if 0
	    std::cerr << "elements_match EINDEX: " << eindex
		      << ", usagepage: " << IOHIDElementGetUsagePage(e)
		      << ", usage: " << IOHIDElementGetUsage(e)
		      << std::endl ;
#endif
	  }
	}
      } else {
	CFArrayRef elements = IOHIDDeviceCopyMatchingElements(device, 0, kIOHIDOptionsTypeNone) ;
	if (elements) {
	  CFIndex ecount = CFArrayGetCount(elements) ;
	  for (CFIndex eindex=0; eindex<ecount; ++eindex) {
	    IOHIDElementRef e = (IOHIDElementRef)CFArrayGetValueAtIndex(elements, eindex) ;
	    IOHIDQueueAddElement(self->theDevice->queue, e) ;
#if 0
	    std::cerr << "!elements_match EINDEX: " << eindex
		      << ", usagepage: " << IOHIDElementGetUsagePage(e)
		      << ", usage: " << IOHIDElementGetUsage(e)
		      << std::endl ;
#endif
	  }
	}
      }
      IOHIDQueueRegisterValueAvailableCallback(self->theDevice->queue, self->queue_callback, self->queue_context) ;
#if USE_CURRENT_RUNLOOP
      CFRunLoopRef runLoop = CFRunLoopGetCurrent() ; 
      CFStringRef runLoopMode = kCFRunLoopDefaultMode ;
#else
      CFRunLoopRef runLoop = CFRunLoopGetMain() ;
      CFStringRef runLoopMode = kCFRunLoopCommonModes ;
#endif
      IOHIDQueueScheduleWithRunLoop(self->theDevice->queue, runLoop, runLoopMode) ;
      IOHIDQueueStart(self->theDevice->queue) ;
    }
  }

  osxHIDInputDevice::osxHIDInputDevice(URI uri,
				       const char *device_description,
                       const char *elements_description):parser(0) {
    theDevice = 0 ;
    inputreport_callback = 0 ;
    inputreport_context = 0 ;
    value_callback = 0 ;
    value_context = 0 ;
    queue_callback = 0 ;
    queue_context = 0 ;
    debugLevel = OSX_DEFAULT_DEBUGLEVEL ;
    seizeDevice = OSX_DEFAULT_SEIZEDEVICE ;

    this->uri = uri ;
    this->uri.generalize() ;
    URI::getQueryArg(uri.query, "debugLevel", &debugLevel) ;
    URI::getQueryArg(uri.query, "seize", &seizeDevice) ;
    parser = new HIDReportParser(NULL, 0, debugLevel);

    manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone) ;
    if (!manager) 
      throw std::runtime_error("IOHIDManagerCreate failed") ;

    device_match = 0 ;
    if (device_description) {
      if (!strncmp(device_description, "<?xml", 5)) {
	device_match = (CFMutableDictionaryRef)getPropertyListFromXML(device_description) ;
	if (debugLevel>1) 
	  std::cerr << "Filtering devices based on XML description: " << device_description << std::endl ;
      } else {
	device_match = (CFMutableDictionaryRef)getPropertyListFromFile(device_description) ;
	if (debugLevel>1) 
	  std::cerr << "Filtering devices based on file " << device_description << std::endl ;
      }
    }
    IOHIDManagerSetDeviceMatching(manager, device_match) ;

    elements_match = 0 ;
    if (elements_description) {
      if (!strncmp(elements_description, "<?xml", 5)) {
	elements_match = (CFArrayRef)getPropertyListFromXML(elements_description) ;
	if (debugLevel>1) 
	  std::cerr << "Filtering elements based on XML description: " << elements_description << std::endl ;
      } else {
	elements_match = (CFArrayRef)getPropertyListFromFile(elements_description) ;
	if (debugLevel>1) 
	  std::cerr << "Filtering elements based on file " << elements_description << std::endl ;
      }
    }
  
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

    IOOptionBits inOptions = seizeDevice ? kIOHIDOptionsTypeSeizeDevice : kIOHIDOptionsTypeNone ;
    if (IOHIDManagerOpen(manager, inOptions)!=kIOReturnSuccess) 
      throw std::runtime_error("IOHIDManagerOpen failed") ;
  }

  void
  osxHIDInputDevice::setInputReportCallback(IOHIDReportCallback callback, void *context) {
    inputreport_callback = callback ;
    inputreport_context = context ;
  }

  void
  osxHIDInputDevice::setValueCallback(IOHIDValueCallback callback, void *context) {
    value_callback = callback ;
    value_context = context ;
  }

  void
  osxHIDInputDevice::setQueueCallback(IOHIDCallback callback, void *context) {
    queue_callback = callback ;
    queue_context = context ;
  }

  URI
  osxHIDInputDevice::getURI(bool expanded) const {
    URI result = uri ;
    if (expanded || seizeDevice!=OSX_DEFAULT_SEIZEDEVICE)
      URI::addQueryArg(result.query, "seize", seizeDevice) ;
    return result ;
  }

  void
  osxHIDInputDevice::RemoveDevice(void *context, IOReturn /*result*/, void */*sender*/, IOHIDDeviceRef device) {
    osxHIDInputDevice *self = (osxHIDInputDevice*)context ;

    if (self->debugLevel>0) std::cerr << "- " << device << std::endl ;

    if (device==self->getDevice()) {
      delete self->theDevice ;
      self->theDevice = 0 ;
    }
  }

  bool
  osxHIDInputDevice::isActive(void) const {
    return theDevice!=0 ;
  }

  IOHIDDeviceRef
  osxHIDInputDevice::getDevice(void) const {
    if (theDevice) return theDevice->device ;
    return 0 ;
  }

  osxHIDInputDevice::~osxHIDInputDevice(void) {
    // std::cerr << "osxHIDInputDevice(" << this << "): deleting theDevice" << std::endl ;
    delete theDevice ;
    delete parser ;

    // std::cerr << "osxHIDInputDevice(" << this << "): deleting device_match" << std::endl ;
    if (device_match) CFRelease(device_match) ;

    // std::cerr << "osxHIDInputDevice(" << this << "): releasing elements_match" << std::endl ;
    if (elements_match) CFRelease(elements_match) ;

    // std::cerr << "osxHIDInputDevice(" << this << "): releasing manager" << std::endl ;
    if (manager) {
      IOHIDManagerClose(manager, kIOHIDOptionsTypeNone) ;
      CFRelease(manager) ;
    }
  }

  // -----------------------------------------------------------------------

}
