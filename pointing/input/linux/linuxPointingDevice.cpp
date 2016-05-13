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

namespace pointing {

  linuxPointingDevice::linuxPointingDevice(URI device_uri):SystemPointingDevice(device_uri)
  {
    URI::getQueryArg(device_uri.query, "seize", &seize);
  }

  URI linuxPointingDevice::getURI(bool expanded, bool crossplatform) const
  {
    URI result = SystemPointingDevice::getURI(expanded, crossplatform);

    if (expanded || seize)
        URI::addQueryArg(result.query, "seize", seize);

    return result;
  }
}
