/* -*- mode: c++ -*-
 *
 * pointing/input/windows/winPointingDeviceManager.cpp --
 *
 * Initial software
 * Authors: Izzatbek Mukhanov
 * Copyright © Inria
 *
 * http://libpointing.org/
 *
 * This software may be used and distributed according to the terms of
 * the GNU General Public License version 2 or any later version.
 *
 */

#include <pointing/input/windows/winPointingDeviceManager.h>
#include <pointing/input/windows/winHIDDeviceDispatcher.h>
#include <pointing/input/windows/USB.h>

using namespace std;

namespace pointing
{
    URI winPointingDeviceManager::uriForHandle(HANDLE h)
    {
        std::stringstream ss;
        if (h)
            ss << "winhid:?handle=0x" << std::hex
               << std::noshowbase << PtrToUint(h);
        else
            ss << "any:";
        return URI(ss.str());
    }

    bool winPointingDeviceManager::ConvertDevice(HANDLE h, PointingDeviceDescriptor &desc)
    {
        int vendorID = -1, productID = -1;
        string vendor, product;
        bool result = getMouseNameFromDevice(h, vendor, product, &vendorID, &productID);
        if (vendorID != -1)
            desc.vendorID = vendorID;
        if (productID != -1)
            desc.productID = productID;
        desc.vendor = vendor;
        desc.product = product;
        desc.devURI = uriForHandle(h);
        return result;
    }

    void winPointingDeviceManager::unregisterMouseDevice(HANDLE h)
    {
        PointingDeviceDescriptor desc;
        desc.devURI = uriForHandle(h);
        removeDevice(desc);
    }

    void winPointingDeviceManager::registerMouseDevice(HANDLE h)
    {
        PointingDeviceDescriptor desc;
        if (ConvertDevice(h, desc))
            addDevice(desc);
    }

    winPointingDeviceManager::winPointingDeviceManager()
    {
		dispatcher = new winHIDDeviceDispatcher(this);
    }

    winPointingDeviceManager::~winPointingDeviceManager()
    {
        delete dispatcher;
    }
}
