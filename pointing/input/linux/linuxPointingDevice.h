/* -*- mode: c++ -*-
 *
 * pointing/input/linux/linuxPointingDevice.h --
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

#ifndef linuxPointingDevice_h
#define linuxPointingDevice_h

#include <pointing/input/PointingDevice.h>

namespace pointing {

  class linuxPointingDevice : public PointingDevice
  {
    friend class linuxPointingDeviceManager;
    URI uri, anyURI;
    
    double forced_cpi, forced_hz;

    int vendorID, productID;
    bool seize;

    int debugLevel;
    PointingCallback callback;
    void *callback_context;

    std::string vendor, product;

    void enableDevice(bool value);
    bool active;

  public:
  
    linuxPointingDevice(URI uri) ;

    bool isActive(void) const;

    int getVendorID(void) const;
    std::string getVendor(void) const;
    int getProductID(void) const;
    std::string getProduct(void) const;

    double getResolution(double *defval=0) const;
    double getUpdateFrequency(double *defval=0) const;

    URI getURI(bool expanded=false, bool crossplatform=false) const;

    void setPointingCallback(PointingCallback callback, void *context=0);

    void setDebugLevel(int level);

    ~linuxPointingDevice();

  } ;

}

#endif
