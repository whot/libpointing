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
//#include <unistd.h>

/*
#include <X11/Xatom.h>
#include <X11/extensions/XInput.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <linux/input.h>
#include <linux/hidraw.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include <iostream>
#include <stdexcept>
*/

namespace pointing {

  #define XORG_DEFAULT_CPI       400.0
  #define XORG_DEFAULT_HZ        125.0
/*
  XDeviceInfo*
  find_device_info(Display	*display,
                   char		*name,
                   Bool		only_extended)
  {
    XDeviceInfo	*devices;
    XDeviceInfo *found = NULL;
    int		loop;
    int		num_devices;
    int		len = strlen(name);
    Bool	is_id = True;
    XID		id = (XID)-1;

    for(loop=0; loop<len; loop++) {
      if (!isdigit(name[loop])) {
        is_id = False;
        break;
      }
    }

    if (is_id) {
      id = atoi(name);
    }

    devices = XListInputDevices(display, &num_devices);

    for(loop=0; loop<num_devices; loop++) {
      if ((!only_extended || (devices[loop].use >= IsXExtensionDevice)) &&
          ((!is_id && strcmp(devices[loop].name, name) == 0) ||
           (is_id && devices[loop].id == id))) {
        if (found) {
          fprintf(stderr,
                  "Warning: There are multiple devices named '%s'.\n"
                  "To ensure the correct one is selected, please use "
                  "the device ID instead.\n\n", name);
          return NULL;
        } else {
          found = &devices[loop];
        }
      }
    }
    return found;
  }

  static inline int str16ToInt(std::string value)
  {
      std::stringstream ss;
      ss << std::hex << value;
      int result;
      ss >> result;
      return result;
  }

  static inline std::string
  bustype2string(int bustype) {
    const char *names[] = {"???", "PCI", "ISAPNP", "USB", "HIL", "BLUETOOTH", "VIRTUAL"} ;
    if (bustype<0 || bustype>BUS_VIRTUAL) return names[0] ;
    return names[bustype] ;
  }

  // --------------------------------------------------------------------------
*/
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
/*
  void linuxPointingDevice::enableDevice(bool value)
  {
    Display	*dpy = XOpenDisplay(0);
    std::string fullName = vendor + " " + product;
    XDeviceInfo *info = find_device_info(dpy, (char *)fullName.c_str(), False);
    if (!info)
    {
      fprintf(stderr, "unable to find the device\n");
      return;
    }

    XDevice *dev = XOpenDevice(dpy, info->id);
    if (!dev)
    {
      fprintf(stderr, "unable to open the device\n");
      return;
    }

    Atom prop = XInternAtom(dpy, "Device Enabled", False);

    unsigned char data = value;

    XChangeDeviceProperty(dpy, dev, prop, XA_INTEGER, 8, PropModeReplace,
                          &data, 1);
    XCloseDevice(dpy, dev);
    XCloseDisplay(dpy);
  }
*/

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
