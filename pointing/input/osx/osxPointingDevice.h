/* -*- mode: c++ -*-
 *
 * pointing/input/osx/osxPointingDevice.h --
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

#ifndef osxPointingDevice_h
#define osxPointingDevice_h

#include <pointing/input/PointingDevice.h>
#include <IOKit/hid/IOHIDManager.h>

namespace pointing {

  class osxPointingDevice : public PointingDevice
  {
    URI uri, anyURI;

    friend class osxPointingDeviceManager;

    PointingCallback callback;
    void *callback_context;

    double forced_cpi, forced_hz;

    int debugLevel;

    bool isUSB(void);
    bool isBluetooth(void);

    IOHIDDeviceRef devRef;
    int vendorID, productID;
    bool seize;

  public:
    osxPointingDevice(URI uri);

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

    ~osxPointingDevice(void);

  };
}

#endif
