/* -*- mode: c++ -*-
 *
 * pointing/input/PointingDeviceManager.h --
 *
 * Initial software
 * Authors: Izzat Mukhanov
 * Copyright © Inria
 *
 * http://libpointing.org/
 *
 * This software may be used and distributed according to the terms of
 * the GNU General Public License version 2 or any later version.
 *
 */

#include <set>
#include <string>
#include <pointing/utils/URI.h>

#ifndef POINTINGDEVICEMANAGER_H
#define POINTINGDEVICEMANAGER_H

namespace pointing
{
    struct PointingDeviceDescriptor
    {
        std::string devURI;
        std::string name;
        //std::string vendor;
        //std::string product;

        int vendorID;
        int productID;

        PointingDeviceDescriptor(std::string devURI = "", std::string name = "")
            :devURI(devURI),name(name),vendorID(0),productID(0)//,vendor("???"),product("???")
        { }

        // To use set of PointingDeviceDescriptors
        bool operator < (const PointingDeviceDescriptor& rhs) const;
    };

    typedef void (*DeviceUpdateCallback)(void *context, const PointingDeviceDescriptor &descriptor, bool wasAdded);
    typedef std::set<PointingDeviceDescriptor> PointingDescriptorSet;

    /**
     * @brief PointingDeviceIterator iterates over the list of pointers to the PointingDevices
     */
    //@{
    typedef PointingDescriptorSet::iterator PointingDescriptorIterator;
    typedef PointingDescriptorSet::const_iterator PointingDescriptorConstIterator;
    //@}

    struct CallbackInfo
    {
        DeviceUpdateCallback callbackFunc;
        void *context;
        CallbackInfo(DeviceUpdateCallback callbackFunc, void *context)
            :callbackFunc(callbackFunc),context(context) { }

        // To use the set of the CallbackInfos
        bool operator < (const CallbackInfo& rhs) const;
    };

    /**
     * @brief The PointingDeviceManager class is a helper class which enumerates
     * the list of existing pointing devices.
     * This class is a singleton which calls its platform-specific subclass
     * constructor.
     *
     * Provides functionality to handle newly added or removed devices.
     */
    class PointingDeviceManager
    {
    protected:
        PointingDeviceManager():callback(NULL) {}
        DeviceUpdateCallback callback;

        virtual ~PointingDeviceManager(void) {}
        static PointingDeviceManager *singleManager;

        PointingDescriptorSet descriptors;

        std::set<CallbackInfo> callbackInfos;
        typedef std::set<CallbackInfo>::iterator CallbackInfoIterator;

        void callCallbackFunctions(PointingDeviceDescriptor &descriptor, bool wasAdded);

    public:

        /**
         * @brief Adds the callback function which is called when
         * a device was added or removed
         */
        void addDeviceUpdateCallback(DeviceUpdateCallback callback, void *context);

        /**
         * @brief Removes the callback function which is called when
         * a device was added or removed
         */
        void removeDeviceUpdateCallback(DeviceUpdateCallback callback, void *context);

        /**
         * @brief This static function is used to instantiate a platform-specific object
         * of the class or return the already existing one.
         */
        static PointingDeviceManager *get();

        /**
         * @brief anyToSpecific Converts a given URI into platform-specific unique URI
         * @param anyURI URI with any scheme
         * @return platform-specific URI
         */
        const URI anyToSpecific(const URI &anyURI) const;

        //static void destroy();

        /**
         * @brief size
         * @return The number of Pointing Devices
         */
        size_t size() const { return descriptors.size(); }

        /*
         * Delegate the iteration to the inner set of the descriptors
         */
        //@{
        PointingDescriptorIterator begin() { return descriptors.begin(); }
        PointingDescriptorIterator end() { return descriptors.end(); }
        //@}

        // TODO: Make this protected
        void addDevice(PointingDeviceDescriptor &descriptor);
        void removeDevice(PointingDeviceDescriptor &descriptor);
    };
}

#endif // POINTINGDEVICEMANAGER_H
