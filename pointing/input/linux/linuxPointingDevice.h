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

#include <pointing/input/SystemPointingDevice.h>

namespace pointing {

  class linuxPointingDevice : public SystemPointingDevice
  {
    friend class linuxPointingDeviceManager;
    bool seize = false;

  public:
    linuxPointingDevice(URI device_uri);
    URI getURI(bool expanded, bool crossplatform) const override;
  };
}

#endif
