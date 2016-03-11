/* -*- mode: c++ -*-
 *
 * pointing/input/PointingDevice.cpp --
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

#include <pointing/utils/URI.h>

#ifndef _MSC_VER // Do not include it on Windows
#include <unistd.h>
#endif

#include <pointing/input/DummyPointingDevice.h>

#ifdef __APPLE__
#include <pointing/input/osx/osxHIDPointingDevice.h>
#endif

#ifdef _WIN32
#include <pointing/input/windows/winPointingDevice.h>
#endif

#ifdef __linux__
#include <pointing/input/linux/linuxHIDPointingDevice.h>
#endif

#include <stdexcept>
#include <iostream>
#include <sstream>
#include <cstdlib>

namespace pointing {

  PointingDevice *
  PointingDevice::create(const char *device_uri) {
    std::string uri ;
    if (device_uri) uri = device_uri ;
    return create(uri) ;
  }

  PointingDevice *
  PointingDevice::create(std::string device_uri) {
    if (device_uri.empty() || device_uri.find("default:")!=std::string::npos) {
      const char *default_device = getenv("POINTING_DEVICE") ;
      device_uri = default_device ? default_device : "" ;
    }

    if (device_uri.empty()) 
      device_uri = (char *)"any:?debugLevel=1" ;

    URI uri(device_uri) ;
    // uri.debug(std::cerr) ;

    bool anywilldo = uri.scheme=="any" ;

#ifdef __APPLE__
    if (anywilldo || uri.scheme=="osxhid")
      return new osxHIDPointingDevice(uri) ;
#endif

#ifdef _WIN32
    if (anywilldo || uri.scheme=="winhid")
      return new winPointingDevice(uri) ;
#endif 

#ifdef __linux__
    if (anywilldo || uri.scheme=="hidraw")
      return new linuxHIDPointingDevice(uri);
#endif

    if (uri.scheme=="dummy")
      return new DummyPointingDevice(uri) ;

    std::stringstream msg ;
    msg << "Unsupported pointing device: \"" << device_uri << "\"" ;
    throw std::runtime_error(msg.str()) ;
  }

  void
  PointingDevice::idle(int milliseconds) {
#ifdef _WIN32
    Sleep(milliseconds) ;
#endif
#ifdef __linux__
    usleep(milliseconds*1000) ;
#endif
#ifdef __APPLE__
    CFRunLoopRunInMode(kCFRunLoopDefaultMode, milliseconds/1000.0, false) ;
#endif 
  }

}
