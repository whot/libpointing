/* -*- mode: c++ -*-
 *
 * pointing/input/linux/linuxPointingDevice.cpp --
 *
 * Initial software
 * Authors: Nicolas Roussel, Izzat Mukhanov
 * Copyright © Inria
 *
 * http://libpointing.org/
 *
 * This software may be used and distributed according to the terms of
 * the GNU General Public License version 2 or any later version.
 *
 */

#include <pointing/input/linux/linuxPointingDevice.h>
#include <pointing/input/linux/linuxPointingDeviceManager.h>

namespace pointing {

  #define XORG_DEFAULT_CPI       400.0
  #define XORG_DEFAULT_HZ        125.0

  linuxPointingDevice::linuxPointingDevice(URI uri):
    uri(uri),forced_cpi(-1.),forced_hz(-1.),
    vendorID(0),productID(0),seize(0),debugLevel(0),
    callback(NULL),callback_context(NULL),active(false)
  {
    URI::getQueryArg(uri.query, "cpi", &forced_cpi);
    URI::getQueryArg(uri.query, "hz", &forced_hz);
    URI::getQueryArg(uri.query, "debugLevel", &debugLevel);
    URI::getQueryArg(uri.query, "seize", &seize);

    linuxPointingDeviceManager *man = (linuxPointingDeviceManager *)PointingDeviceManager::get();

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

  bool linuxPointingDevice::isActive(void) const {
    return active;
  }

  int linuxPointingDevice::getVendorID() const
  {
      return vendorID;
  }

  std::string linuxPointingDevice::getVendor() const
  {
      return vendor;
  }

  int linuxPointingDevice::getProductID() const
  {
      return productID;
  }

  std::string linuxPointingDevice::getProduct() const
  {
      return product;
  }

  double linuxPointingDevice::getResolution(double *defval) const {
    if (forced_cpi > 0) return forced_cpi;
    return defval ? *defval : XORG_DEFAULT_CPI;
  }

  double linuxPointingDevice::getUpdateFrequency(double *defval) const {
    if (forced_hz > 0) return forced_hz;
    double estimated = estimatedUpdateFrequency();
    if (estimated > 0.)
      return estimated;
    return defval ? *defval : XORG_DEFAULT_HZ;
  }

  URI linuxPointingDevice::getURI(bool expanded, bool crossplatform) const
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

  void linuxPointingDevice::setPointingCallback(PointingCallback cbck, void *ctx)
  {
    callback = cbck;
    callback_context = ctx;
  }

  void linuxPointingDevice::setDebugLevel(int level)
  {
    debugLevel = level;
  }

  linuxPointingDevice::~linuxPointingDevice()
  {
    linuxPointingDeviceManager *man = (linuxPointingDeviceManager *)PointingDeviceManager::get();
    man->removePointingDevice(this);
  }
}
