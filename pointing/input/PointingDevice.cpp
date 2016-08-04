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
#include <math.h>

namespace pointing {

#define errThreshold .6

  void PointingDevice::registerTimestamp(TimeStamp::inttime timestamp, int dx, int dy)
  {
    if (!(dx || dy))
      return;

    double delta = double(timestamp - lastTime) / TimeStamp::one_millisecond;
    lastTime = timestamp;
    // Delta cannot be less than 0.4 ms
    // So filter it out
    if (delta < 0.4)
      delta = dxInd;

    sumDx += delta - dxs[dxInd];
    dxs[dxInd] = delta;
    dxInd = (dxInd + 1) % N;

    double average = sumDx / N;
    double variance = 0.;
    for (int i = 0; i < N; i++)
    {
        double dif = dxs[i] - average;
        variance += dif * dif;
    }

    double curEstimate = sumDx / N;
    // Polling rate was changed
    if (variance < stableVariance && curEstimate - estimate > errThreshold)
    {
      // Reset minimum variance
      minVariance = 10e9;
    }
    // Improve estimate
    if (variance < minVariance)
    {
      minVariance = variance;
      if (curEstimate - estimate > errThreshold && minVariance < stableVariance)
      {
        stableVariance = minVariance;
      }
      estimate = curEstimate;
    }
  }

  double PointingDevice::estimatedUpdateFrequency() const
  {
    if (minVariance >= stableVariance)
      return -1.;

    static const int stdFreqN = 4;
    static const double stdFreqs[stdFreqN] = { 1., 2., 4., 8. };

    for (int i = 0; i < stdFreqN; i++)
    {
      double err = fabs(estimate - stdFreqs[i]);
      if (err < errThreshold)
        return 1000. / stdFreqs[i];
    }
    return 1000. / estimate;
  }

  PointingDevice::PointingDevice()
  {
    for (int i = 0; i < N; i++)
      dxs[i] = 0;
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
