/* -*- mode: c++ -*-
 *
 * pointing/output/osx/osxDisplayDeviceManager.cpp --
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

#include <pointing/output/osx/osxDisplayDeviceManager.h>
#include <IOKit/graphics/IOGraphicsLib.h>
#include <stdexcept>

extern "C" bool NSApplicationLoad(void) ;

// https://developer.apple.com/library/mac/documentation/GraphicsImaging/Reference/Quartz_Services_Ref/index.html#//apple_ref/doc/c_ref/CGDisplayChangeSummaryFlags

namespace pointing
{
  void osxDisplayDeviceManager::MyDisplayReconfigurationCallBack(CGDirectDisplayID display, CGDisplayChangeSummaryFlags flags, void *userInfo) {
    osxDisplayDeviceManager *self = (osxDisplayDeviceManager *)userInfo;
    if (flags & kCGDisplayAddFlag)
      self->addDisplay(display);
    if (flags & kCGDisplayRemoveFlag)
      self->removeDisplay(display);
  }

  std::string CFStringRefToStdString(CFStringRef aString) {
    if (aString == NULL) {
      return "Unknown";
    }

    CFIndex length = CFStringGetLength(aString);
    CFIndex maxSize =
    CFStringGetMaximumSizeForEncoding(length,
                                      kCFStringEncodingUTF8);
    char *buffer = (char *)malloc(maxSize);
    if (CFStringGetCString(aString, buffer, maxSize,
                           kCFStringEncodingUTF8)) {
      std::string result(buffer);
      free(buffer);
      return result;
    }
    return "Unknown";
  }

  std::string getDisplayName(CGDirectDisplayID did)
  {
    std::string result;
    CFDictionaryRef deviceInfo = IODisplayCreateInfoDictionary(CGDisplayIOServicePort(did), kIODisplayOnlyPreferredName);
    CFTypeRef value = NULL;
    value = CFDictionaryGetValue(deviceInfo, CFSTR(kDisplayProductName));
    if (value)
    {
      CFDictionaryRef valueDict = (CFDictionaryRef)value;
      CFIndex number = CFDictionaryGetCount(valueDict);
      if (number > 0)
      {
        CFStringRef *key = (CFStringRef *)malloc(sizeof(CFStringRef)*number);
        CFDictionaryGetKeysAndValues(valueDict, NULL, (const void **)key);
        result = CFStringRefToStdString(key[0]);
        free(key);
      }
    }
    else
      result = "Unknown Display Name";
    CFRelease(deviceInfo);
    return result;
  }

  DisplayDeviceDescriptor osxDisplayDeviceManager::convertDevice(CGDirectDisplayID did)
  {
    std::stringstream uri;
    uri << "osxdisplay:/" << did;
    DisplayDeviceDescriptor desc;
    desc.devURI = uri.str();
    desc.name = getDisplayName(did);
    return desc;
  }

  void osxDisplayDeviceManager::addDisplay(CGDirectDisplayID did)
  {
    DisplayDeviceDescriptor desc = convertDevice(did);
    addDevice(desc);
  }

  void osxDisplayDeviceManager::removeDisplay(CGDirectDisplayID did)
  {
    DisplayDeviceDescriptor desc = convertDevice(did);
    removeDevice(desc);
  }

  osxDisplayDeviceManager::osxDisplayDeviceManager()
  {
    CGDisplayCount dspyCnt = 0 ;
    CGDisplayErr err = CGGetActiveDisplayList(0, 0, &dspyCnt) ;
    CGDirectDisplayID *activeDspys = new CGDirectDisplayID [dspyCnt] ;
    err = CGGetActiveDisplayList(dspyCnt, activeDspys, &dspyCnt) ;
    for (unsigned int i = 0; i < dspyCnt; ++i) {
      DisplayDeviceDescriptor desc = convertDevice(activeDspys[i]) ;
      addDevice(desc);
    }
    delete [] activeDspys;
    NSApplicationLoad() ;
    CGDisplayRegisterReconfigurationCallback(MyDisplayReconfigurationCallBack, this);
  }
}
