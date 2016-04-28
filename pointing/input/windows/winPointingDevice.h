/* -*- mode: c++ -*-
 *
 * pointing/input/windows/winHIDPointingDevice.h --
 *
 * Initial software
 * Authors: Damien Marchal, Izzat Mukhanov
 * Copyright © INRIA
 *
 */

#ifndef winPointingDevice_h
#define winPointingDevice_h

#include <pointing/input/PointingDevice.h>

#include <windows.h>

namespace pointing
{
    /**
     * @brief The winPointingDevice class is a platform-specific subclass of PointingDevice,
     * based on the RawInput API, recommended by Microsoft.
     * This is the input method with the largest number of functionalities
     */
    class winPointingDevice : public PointingDevice
    {
        PointingCallback callback ;
        void *callback_context ;

        void setActive(HANDLE h, bool isActive);
        friend class winHIDDeviceDispatcher;

    protected:

        URI uri ;

        int debugLevel;
        double forced_cpi;
        double forced_hz;
        unsigned int handle;

        int vendorID, productID;
        std::string vendor, product;
        URI anyURI; // Used only if a given URI has any: scheme
        int buttons;

        bool active;

        // For absolute coordinates
        int lastX, lastY;

    public:

        winPointingDevice(URI device_uri) ;

        bool isActive(void) const ;

        int getVendorID(void) const ;
        std::string getVendor(void) const ;
        int getProductID(void) const ;
        std::string getProduct(void) const ;

        URI getURI(bool expanded=false, bool crossplatform=false) const ;

        double getResolution(double *defval=0) const ;
        double getUpdateFrequency(double *defval=0) const ;

        void setPointingCallback(PointingCallback callback, void *context=0) ;
        void setDebugLevel(int level) { debugLevel = level ; }

        ~winPointingDevice(void) ;
    } ;
}

#endif
