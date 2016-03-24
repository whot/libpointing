/* -*- mode: c++ -*-
 *
 * pointing/input/windows/winPointingDevice.cpp --
 *
 * Initial software
 * Authors: Damien Marchal, Izzat Mukhanov
 * Copyright © Inria
 *
 * http://libpointing.org/
 *
 * This software may be used and distributed according to the terms of
 * the GNU General Public License version 2 or any later version.
 *
 */
/* See file LICENSE in the top-directory of the project. */


#include <pointing/input/windows/winPointingDevice.h>
#include <pointing/input/windows/USB.h>
#include <pointing/input/windows/winPointingDeviceManager.h>
#include <pointing/input/windows/winHIDDeviceDispatcher.h>

namespace pointing {

#define WIN_DEFAULT_CPI         400
#define WIN_DEFAULT_HZ          125
#define WIN_DEFAULT_DEBUGLEVEL  0
#define WIN_DEFAULT_HANDLE      0

    URI uriForHandle(HANDLE h)
    {
        std::stringstream ss;
        ss << "winhid:?handle=0x" << std::hex << std::noshowbase << PtrToUint(h);
        return URI(ss.str());
    }

    void winPointingDevice::setActive(HANDLE h, bool isActive)
    {
      if (h == (HANDLE)handle)
      {
        active = isActive;
        if (active)
        {
          int vendorID = 0, productID = 0;
          std::string vendor = "", product = "";
          if (getMouseNameFromDevice((HANDLE)handle, vendor, product, &vendorID, &productID))
          {
            this->vendor = vendor;
            this->product = product;
            this->vendorID = vendorID;
            this->productID = productID;
          }
        }
      }
      if (!active && !anyURI.asString().empty()) // Try to find another matching device
      {
        winPointingDeviceManager *man = (winPointingDeviceManager *)PointingDeviceManager::get();
        URI newUri = man->anyToSpecific(anyURI);
        if (uri != newUri && anyURI != newUri) // Another matching device was found
        {
          man->dispatcher->removePointingDevice((HANDLE)handle, this);
          this->uri = newUri;
          ATTRIB_FROM_URI(uri.query, handle);
          man->dispatcher->addPointingDevice((HANDLE)handle, this);
        }
      }
    }

    winPointingDevice::winPointingDevice(URI uri) :
        debugLevel(WIN_DEFAULT_DEBUGLEVEL),forced_cpi(-1.),
        forced_hz(-1.),handle(WIN_DEFAULT_HANDLE),
        active(0),vendorID(0),productID(0),buttons(0)
    {
      try {
        callback=NULL;
        callback_context=NULL;

        ATTRIB_FROM_URI(uri.query,debugLevel);
        ATTRIB_FROM_URI(uri.query,forced_cpi);
        ATTRIB_FROM_URI(uri.query,forced_hz);

        winPointingDeviceManager *man = (winPointingDeviceManager *)PointingDeviceManager::get();

        if (uri.scheme == "any")
        {
          anyURI = uri;
          uri = man->anyToSpecific(anyURI);
        }
        this->uri = uri;

        ATTRIB_FROM_URI(uri.query, handle);
        man->dispatcher->addPointingDevice((HANDLE)handle, this);

        if (debugLevel)
        {
          for (PointingDescriptorIterator it = man->begin(); it != man->end(); it++)
          {
              PointingDeviceDescriptor desc = *it;
              unsigned curHandle = 0;
              URI::getQueryArg(URI(desc.devURI).query, "handle", &curHandle);
              bool match = (handle == curHandle);
              std::cout << (match ? "+ " : "  ") << desc.devURI
                   << " [" << std::hex << "vend:0x" << desc.vendorID
                   << ", prod:0x" << desc.productID
                   << std::dec << " - " << desc.name << " ]" << std::endl;
          }
        }

      } catch (std::runtime_error e) {
        std::cerr << "Runtime error: " << e.what() << std::endl ;
      } catch (std::exception e) {
        std::cerr << "Exception: " << e.what() << std::endl ;
      }
    }

    double
    winPointingDevice::getResolution(double *defval) const {
      if (forced_cpi > 0)
        return forced_cpi;
      return defval ? *defval : WIN_DEFAULT_CPI ;
    }

    double
    winPointingDevice::getUpdateFrequency(double *defval) const {
      if (forced_hz > 0)
        return forced_hz;
      double estimated = estimatedUpdateFrequency();
      if (estimated > 0)
        return estimated;
      return defval ? *defval : WIN_DEFAULT_HZ ;
    }

    void
    winPointingDevice::setPointingCallback(PointingCallback cbck, void *ctx) {
      // Thread-issue, to guarantee that the callback will be call with a not yet
      // initialized context we first set the context then the callback.
      callback_context = ctx ; 
      callback = cbck ;
    }

    URI
    winPointingDevice::getURI(bool expanded, bool crossplatform) const {
      URI result;

      if (crossplatform)
      {
          result.scheme = "any";
          if (vendorID)
              URI::addQueryArg(result.query, "vendor", vendorID);
          if (productID)
              URI::addQueryArg(result.query, "product", productID);
      }
      else
          result = uri;

      if (expanded || debugLevel)
          URI::addQueryArg(result.query, "debugLevel", debugLevel);
      if (expanded || forced_cpi > 0)
          URI::addQueryArg(result.query, "cpi", forced_cpi);
      if (expanded || forced_hz > 0)
          URI::addQueryArg(result.query, "hz", forced_hz);

      return result;
    }

    bool
    winPointingDevice::isActive(void) const {
        return active;
    }

    int winPointingDevice::getVendorID() const
    {
        return this->vendorID;
    }

    std::string winPointingDevice::getVendor() const
    {
        return this->vendor;
    }

    int winPointingDevice::getProductID() const
    {
        return this->productID;
    }

    std::string winPointingDevice::getProduct() const
    {
        return this->product;
    }

    winPointingDevice::~winPointingDevice(void) {
      winPointingDeviceManager *man = (winPointingDeviceManager *)PointingDeviceManager::get();
      man->dispatcher->removePointingDevice((HANDLE)handle, this);
      //DestroyWindow(msghwnd_);
    }
}
