/* -*- mode: c++ -*-
 *
 * pointing/input/linux/linuxHIDPointingDevice.h --
 *
 * Initial software
 * Authors: Nicolas Roussel
 * Copyright © Inria
 *
 * http://libpointing.org/
 *
 * This software may be used and distributed according to the terms of
 * the GNU General Public License version 2 or any later version.
 *
 */

#ifndef linuxHIDPointingDevice_h
#define linuxHIDPointingDevice_h

#include <pointing/input/PointingDevice.h>
#include <pointing/utils/HIDReportParser.h>

#include <pthread.h>
#include <libudev.h>

struct hidraw_report_descriptor;


namespace pointing {

  /**
   * @brief Linux-specific pointing device class which is based on libudev.
   *
   * Libudev is used to inspect the devices, in this case hidraw devices.
   * It can access their Vendor ID (VID), Product ID (PID), serial number,
   * and device strings, without ever opening the device.
   * http://www.kernel.org/pub/linux/utils/kernel/hotplug/libudev/
   *
   */
  class linuxHIDPointingDevice : public PointingDevice {

    /**
     * @brief Handle to the udev
     */
    struct udev *udev ;
    struct udev_monitor *monitor ;

    /**
     * @brief Id associated with the device
     */
    int hid ;
    URI uri ;
    int debugLevel ;
    PointingCallback callback ;
    void *callback_context ;
    
    double forced_cpi, forced_hz ;

    int vendorID, productID;
    std::string vendor, product;

    HIDReportParser *parser;
    int reportLength;

    bool seizeDevice;
    void enableDevice(bool value);

    pthread_t thread ;

    static void *eventloop(void *self) ;

    /**
     * @brief The monitoring interface will report events to the application when the status of a device changes.
     *
     * This is useful for receiving notification when devices are connected or disconnected from the system.
     * Then a device changes state, the udev_monitor_receive_device()
     * function will return a handle to a udev_device which represents the object which was changed.
     *
     * The returned object can then be queried with the udev_device_get_action() function to determine which action occurred.
     *
     * The actions are returned as the following strings:
     *    add - Device is connected to the system
     *    remove - Device is disconnected from the system
     */
    void monitor_readable() ;

    /**
     * @brief If the new hidraw device was found
     * this method is called to read the device info
     */
    void checkFoundDevice(struct udev_device *device) ;

    /**
     * @brief If the device was removed then hid = -1
     */
    void checkLostDevice(struct udev_device *device) ;

    /**
     * @brief Reads the Device events and calls the callback function.
     */
    void hid_readable() ;
    bool applyquirck;
  public:
  
    linuxHIDPointingDevice(URI uri) ;

    bool isActive(void) const ;

    int getVendorID(void) const ;
    std::string getVendor(void) const ;
    int getProductID(void) const ;
    std::string getProduct(void) const ;

    double getResolution(double *defval=0) const ;
    double getUpdateFrequency(double *defval=0) const ;

    URI getURI(bool expanded=false, bool crossplatform=false) const ;

    void setPointingCallback(PointingCallback callback, void *context=0) ;

    void setDebugLevel(int level) ;

    ~linuxHIDPointingDevice() ;

  } ;

}

#endif
