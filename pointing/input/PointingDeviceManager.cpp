/* -*- mode: c++ -*-
 *
 * pointing/input/PointingDeviceManager.cpp --
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

#ifdef __APPLE__
#include <pointing/input/osx/osxPointingDeviceManager.h>
#endif

#ifdef _WIN32
#include <pointing/input/windows/winPointingDeviceManager.h>
#endif

#ifdef __linux__
#include <pointing/input/linux/linuxPointingDeviceManager.h>
#endif

#include <stdexcept>
#include <iostream>
#include <pointing/input/PointingDeviceManager.h>

namespace pointing {

#define  DEFAULT_VENDOR    0
#define  DEFAULT_PRODUCT   0

    PointingDeviceManager *PointingDeviceManager::singleManager = 0;

    PointingDeviceManager *PointingDeviceManager::get()
    {
        if (singleManager == NULL)
        {
        #ifdef __APPLE__
            singleManager = new osxPointingDeviceManager();
            //singleManager = new PointingDeviceManager();
        #endif

        #ifdef _WIN32
            singleManager = new winPointingDeviceManager();
        #endif

        #ifdef __linux__
            singleManager = new linuxPointingDeviceManager();
        #endif
        }
        return singleManager;
    }

    bool PointingDeviceDescriptor::operator < (const PointingDeviceDescriptor& rhs) const
    {
        return devURI.asString() < rhs.devURI.asString();
    }

    // To use the set of the CallbackInfos
    bool CallbackInfo::operator < (const CallbackInfo& rhs) const
    {
        if (context < rhs.context) return true;
        if (context > rhs.context) return false;

        return callbackFunc < rhs.callbackFunc;
    }

    void PointingDeviceManager::callCallbackFunctions(PointingDeviceDescriptor &descriptor, bool wasAdded)
    {
        for (CallbackInfoIterator it = callbackInfos.begin(); it != callbackInfos.end(); it++)
        {
            CallbackInfo callbackInfo = *it;
            callbackInfo.callbackFunc(callbackInfo.context, descriptor, wasAdded);
        }
    }

    void PointingDeviceManager::addDevice(PointingDeviceDescriptor &descriptor)
    {
        descriptors.insert(descriptor);
        callCallbackFunctions(descriptor, true);
    }

    void PointingDeviceManager::removeDevice(PointingDeviceDescriptor &descriptor)
    {
        PointingDescriptorIterator it = descriptors.find(descriptor);
        if (it != descriptors.end())
        {
            PointingDeviceDescriptor foundDesc = *it;
            descriptors.erase(it);
            callCallbackFunctions(foundDesc, false);
        }
    }

    URI PointingDeviceManager::anyToSpecific(const URI &anyURI) const
    {
        if (anyURI.scheme != "any")
        {
            std::cerr << "PointingDeviceManager::anyToSpecific: URI scheme must be \"any\"" << std::endl;
            return anyURI;
        }
        int vendorID = DEFAULT_VENDOR;
        int productID = DEFAULT_PRODUCT;
        URI::getQueryArg(anyURI.query, "vendor", &vendorID);
        URI::getQueryArg(anyURI.query, "product", &productID);

        for (PointingDescriptorIterator it = descriptors.begin(); it != descriptors.end(); it++)
        {
            PointingDeviceDescriptor pdd = *it;
            if ((!vendorID || pdd.vendorID == vendorID)
             && (!productID || pdd.productID == productID))
            {
                return pdd.devURI;
            }
        }
        //std::cerr << "Warning: could not find a device with a given URI" << std::endl ;
        return anyURI;
    }

    URI PointingDeviceManager::generalizeAny(const URI &anyURI)
    {
      URI result = anyURI;
      int vendorID = DEFAULT_VENDOR, productID = DEFAULT_PRODUCT;
      URI::getQueryArg(anyURI.query, "vendor", &vendorID);
      URI::getQueryArg(anyURI.query, "product", &productID);
      result.generalize();
      if (vendorID != DEFAULT_VENDOR)
        URI::addQueryArg(result.query, "vendor", vendorID);
      if (productID != DEFAULT_PRODUCT)
        URI::addQueryArg(result.query, "product", productID);
      return result;
    }

    /*void PointingDeviceManager::destroy()
    {
        delete singleManager;
        singleManager = NULL;
    }
    */

    void PointingDeviceManager::addDeviceUpdateCallback(DeviceUpdateCallback callback, void *context)
    {
        CallbackInfo info(callback, context);
        callbackInfos.insert(info);
    }

    void PointingDeviceManager::removeDeviceUpdateCallback(DeviceUpdateCallback callback, void *context)
    {
        CallbackInfo info(callback, context);
        callbackInfos.erase(info);
    }
}

