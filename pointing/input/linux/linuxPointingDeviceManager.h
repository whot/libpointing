/* -*- mode: c++ -*-
 *
 * pointing/input/linux/linuxPointingDeviceManager.h --
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

#ifndef linuxPointingDeviceManager_h
#define linuxPointingDeviceManager_h

#include <pointing/input/PointingDeviceManager.h>
#include <pointing/input/linux/linuxPointingDevice.h>
#include <pthread.h>
#include <libudev.h>
#include <pointing/utils/HIDReportParser.h>

namespace pointing {

  /**
   * @brief The linuxPointingDeviceManager class is the platform-specific
   * subclass of the PointingDeviceManager class
   */
  class linuxPointingDeviceManager : public PointingDeviceManager
  {
    friend class PointingDeviceManager;
    friend class linuxPointingDevice;

    // Add linux-specific data
    struct linuxPointingDeviceData : PointingDeviceData
    {
      HIDReportParser parser;
      int devID;
      int reportLength;
      pthread_t thread;

      // To seize a given device
      std::string evDevNode;
      int evDevId = -1;
      // If there are several PointingDevice objects with seize
      // corresponding to the same physical device
      // Seize the device until all of them are deleted
      int seizeCount = 0;
    };

    struct udev *udev ;
    struct udev_monitor *monitor ;

    pthread_t thread ;

    /**
     * @brief This static function works in another thread.
     * It queries for added or removed devices.
     */
    static void *eventloop(void *self);
    static void *checkReports(void *self);

    void enableDevice(bool value, std::string fullName);

    void monitor_readable();
    void hid_readable(linuxPointingDeviceData *pdd);

    int readHIDDescriptor(int devID, HIDReportParser *parser);
    void fillDescriptorInfo(struct udev_device *hiddev, struct udev_device *usbdev, PointingDeviceDescriptor &desc);

    void processMatching(PointingDeviceData *pdd, SystemPointingDevice *device);

    void checkFoundDevice(struct udev_device *device);
    void checkLostDevice(struct udev_device *device);

    void unSeizeDevice(linuxPointingDeviceData *data);
    virtual void removePointingDevice(SystemPointingDevice *device) override;

    linuxPointingDeviceManager();
    ~linuxPointingDeviceManager();
  };

}

#endif
