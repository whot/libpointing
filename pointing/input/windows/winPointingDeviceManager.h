/* -*- mode: c++ -*-
 *
 * pointing/input/windows/winPointingDeviceManager.h --
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
#ifndef WINPOINTINGDEVICEMANAGER_H
#define WINPOINTINGDEVICEMANAGER_H

#include <pointing/input/PointingDeviceManager.h>
#include <windows.h>
#include <vector>

namespace pointing
{
    class winPointingDeviceManager : public PointingDeviceManager
    {
        friend class winHIDDeviceDispatcher;
        bool ConvertDevice(HANDLE h, PointingDeviceDescriptor &desc);

        void registerMouseDevice(HANDLE, RID_DEVICE_INFO&);
        void unregisterMouseDevice(HANDLE h);

        winHIDDeviceDispatcher *dispatcher;

        friend class PointingDeviceManager;
        friend class winPointingDevice;

        winPointingDeviceManager();
        ~winPointingDeviceManager();
    };
}

#endif // WINPOINTINGDEVICEMANAGER_H
