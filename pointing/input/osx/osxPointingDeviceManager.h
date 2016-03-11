/* -*- mode: c++ -*-
 *
 * pointing/input/osx/osxPointingDeviceManager.h --
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
#ifndef osxPointingDeviceManager_h
#define osxPointingDeviceManager_h

#include <iostream>
#include <map>
#include <pointing/input/PointingDeviceManager.h>
#include <IOKit/hid/IOHIDManager.h>


namespace pointing
{
    /**
     * @brief The osxPointingDeviceManager class is the platform-specific
     * subclass of the PointingDeviceManager class.
     *
     * There is no public members of this class, because all the functions are called by its parent
     * which is also a friend of this class.
     */
    class osxPointingDeviceManager : public PointingDeviceManager
    {
        friend class PointingDeviceManager;
        typedef std::map<IOHIDDeviceRef, PointingDeviceDescriptor> descMap_t;

        // Map is needed because we cannot find all the information about removed device
        descMap_t descMap;

        IOHIDManagerRef manager;
        static void AddDevice(void *context, IOReturn /*result*/, void *sender, IOHIDDeviceRef device);
        static void RemoveDevice(void *context, IOReturn /*result*/, void *sender, IOHIDDeviceRef device);
        static void ConvertDevice(IOHIDDeviceRef inDevice, PointingDeviceDescriptor &desc);
        osxPointingDeviceManager();
        ~osxPointingDeviceManager() {}
    };
}


#endif
