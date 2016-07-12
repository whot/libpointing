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
#include <CoreFoundation/CoreFoundation.h>
#include <pointing/input/osx/osxPointingDevice.h>
#endif
#ifdef __linux__
#include <pointing/input/linux/linuxPointingDevice.h>
#endif
#ifdef _WIN32
#include <windows.h>
#include <pointing/input/windows/winPointingDevice.h>
#endif

#include <stdexcept>
#include <iostream>
#include <sstream>
#include <cstdlib>

namespace pointing {

  typedef enum {
    FREQUENCY_LOW,
    FREQUENCY_125,
    FREQUENCY_250,
    FREQUENCY_500,
    FREQUENCY_1000
  } DeviceFrequency;

  // Each delta increases the corresponding value of buckets-accumulator
  // For example:
  // 0.87 ms -> buckets[FREQUENCY_1000]++
  // 1.8 ms -> buckets[FREQUENCY_500]++
  // 3.4 ms -> buckets[FREQUENCY_250]++
  // everything > 14 ms -> buckets[FREQUENCY_LOW]++
  // which is normally an outlier and produced at the beginning of a movement
  void PointingDevice::registerTimestamp(TimeStamp::inttime timestamp)
  {
    static int minThresholds[BUCKETS_SIZE] = {14000, 6000, 3000, 1500, 0};
    TimeStamp::inttime delta = (timestamp - lastTime) / TimeStamp::one_microsecond;
    lastTime = timestamp;
    for (int i = 0; i < BUCKETS_SIZE; i++)
    {
      if (delta > minThresholds[i])
      {
        buckets[i] += 1;
        return;
      }
    }
  }

  double PointingDevice::estimatedUpdateFrequency() const
  {
    static const int MIN_N = 25;
    static const double minPerc[BUCKETS_SIZE] = {0.5, 0.4, 0.3, 0.2, 0.1};
    unsigned long sum = 0;
    for (int i = 1; i < BUCKETS_SIZE; i++) {
      sum += buckets[i];
    }
    if (sum < MIN_N)
      return -1;
    if (double(buckets[FREQUENCY_1000]) / sum > minPerc[FREQUENCY_1000])
      return 1000.;
    if (double(buckets[FREQUENCY_500]) / sum > minPerc[FREQUENCY_500])
      return 500.;
    if (double(buckets[FREQUENCY_250]) / sum > minPerc[FREQUENCY_250])
      return 250.;
    if (double(buckets[FREQUENCY_125]) / sum > minPerc[FREQUENCY_125])
      return 125.;
    return -1;
  }

  PointingDevice::PointingDevice()
    :lastTime(0) {
    memset(buckets, 0, sizeof(buckets));
  }

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
      return new osxPointingDevice(uri);
#endif
#ifdef _WIN32
    if (anywilldo || uri.scheme=="winhid")
      return new winPointingDevice(uri) ;
#endif
#ifdef __linux__
    if (anywilldo || uri.scheme=="input")
      return new linuxPointingDevice(uri) ;
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

  void PointingDevice::getAbsolutePosition(double *x, double *y) const
  {
    *x = -1;
    *y = -1;
  }

}
