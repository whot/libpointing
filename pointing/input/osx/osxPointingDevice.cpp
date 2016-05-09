/* -*- mode: c++ -*-
 *
 * pointing/input/osx/osxPointingDevice.cpp --
 *
 * Initial software
 * Authors: Nicolas Roussel, Izzatbek Mukhanov
 * Copyright © Inria
 *
 * http://libpointing.org/
 *
 * This software may be used and distributed according to the terms of
 * the GNU General Public License version 2 or any later version.
 *
 */

#include <pointing/input/osx/osxPointingDevice.h>
#include <pointing/input/osx/osxHIDUtils.h>
#include <pointing/input/osx/osxPointingDeviceManager.h>

namespace pointing {

#define OSX_DEFAULT_CPI           400.001
#define OSX_DEFAULT_HZ            125.001

  osxPointingDevice::osxPointingDevice(URI uri)
     :uri(uri),callback(NULL),callback_context(0),
      forced_cpi(-1.),forced_hz(-1.),debugLevel(0),
      devRef(0),vendorID(0),productID(0),seize(false)
  {
    URI::getQueryArg(uri.query, "cpi", &forced_cpi);
    URI::getQueryArg(uri.query, "hz", &forced_hz);
    URI::getQueryArg(uri.query, "debugLevel", &debugLevel);
    URI::getQueryArg(uri.query, "seize", &seize);

    osxPointingDeviceManager *man = (osxPointingDeviceManager *)PointingDeviceManager::get();

    if (uri.scheme == "any")
    {
      anyURI = PointingDeviceManager::generalizeAny(uri);
      URI::getQueryArg(uri.query, "vendor", &vendorID);
      URI::getQueryArg(uri.query, "product", &productID);
    }
    else
      this->uri.generalize();

    man->addPointingDevice(this);
  }

  bool osxPointingDevice::isActive(void) const {
    return devRef != NULL;
  }

  URI osxPointingDevice::getURI(bool expanded, bool crossplatform) const
  {
    URI result = uri;
    if (crossplatform)
    {
      if (anyURI.scheme.size())
        result = anyURI;
      else
      {
        int vendorID = getVendorID();
        if (vendorID)
          URI::addQueryArg(result.query, "vendor", vendorID) ;

        int productID = getProductID();
        if (productID)
          URI::addQueryArg(result.query, "product", productID) ;
      }
    }

    if (expanded || seize)
        URI::addQueryArg(result.query, "seize", seize);
    if (expanded || debugLevel)
        URI::addQueryArg(result.query, "debugLevel", debugLevel);
    if (expanded || forced_cpi > 0)
        URI::addQueryArg(result.query, "cpi", getResolution());
    if (expanded || forced_hz > 0)
        URI::addQueryArg(result.query, "hz", getUpdateFrequency());

    return result;
  }

  bool osxPointingDevice::isUSB(void)
  {
    return uri.path.find("/USB") == 0;
  }

  bool osxPointingDevice::isBluetooth(void)
  {
    return uri.path.find("/Bluetooth") == 0;
  }

  int osxPointingDevice::getVendorID(void) const
  {
    if (devRef)
      return hidDeviceGetIntProperty(devRef, CFSTR(kIOHIDVendorIDKey));
    return vendorID;
  }

  std::string osxPointingDevice::getVendor(void) const
  {
    if (devRef)
      return hidDeviceGetStringProperty(devRef, CFSTR(kIOHIDManufacturerKey));
    return "???";
  }

  int osxPointingDevice::getProductID(void) const
  {
    if (devRef)
      return hidDeviceGetIntProperty(devRef, CFSTR(kIOHIDProductIDKey));
    return productID;
  }

  std::string osxPointingDevice::getProduct(void) const
  {
    if (devRef)
      return hidDeviceGetStringProperty(devRef, CFSTR(kIOHIDProductKey));
    return "???";
  }

  double osxPointingDevice::getResolution(double *defval) const
  {
    if (forced_cpi > 0)
      return forced_cpi;

    if (devRef)
    {
      double result = -1.0;
      result = hidGetPointingResolution(devRef);
      if (result > 0)
        return result;
    }

    if (defval)
      return *defval;

    return OSX_DEFAULT_CPI;
  }

  double osxPointingDevice::getUpdateFrequency(double *defval) const
  {
    if (forced_hz > 0)
      return forced_hz;

    if (devRef)
    {
      double result = -1;
      result = 1.0 / hidGetReportInterval(devRef);
      double estimated = estimatedUpdateFrequency();
      if (result == 125. && estimated > 0.)
        return estimated;
      if (result > 0)
        return result;
      if (estimated > 0.)
        return estimated;
    }

    if (defval)
      return *defval;

    return OSX_DEFAULT_HZ;
  }

  void osxPointingDevice::setDebugLevel(int level)
  {
    this->debugLevel = level;
  }

  void osxPointingDevice::setPointingCallback(PointingCallback cbck, void *ctx)
  {
    callback = cbck;
    callback_context = ctx;
  }

  osxPointingDevice::~osxPointingDevice(void)
  {
    osxPointingDeviceManager *man = (osxPointingDeviceManager *)PointingDeviceManager::get();
    man->removePointingDevice(this);
  }
}
