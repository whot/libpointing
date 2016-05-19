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

    struct linuxPointingDeviceData : PointingDeviceData
    {
      int devID = -1;
      pthread_t thread;
      std::string evDevPath;
      int evDevId = -1;
      int seizeCount = 0;
    };

    struct udev *udev;
    struct udev_monitor *monitor;

    pthread_t thread;

    /**
     * @brief This static function works in another thread.
     * It queries for added or removed devices.
     */
    static void *eventloop(void *self);
    static void *checkReports(void *self);

    void enableDevice(bool value, std::string fullName);

    void monitor_readable();
    void readable(linuxPointingDeviceData *pdd);

    int readHIDDescriptor(int devID, HIDReportParser *parser);
    void fillExternalDescInfo(udev_device *hiddev, udev_device *usbdev, PointingDeviceDescriptor &desc);
    void fillEmbeddedDescInfo(udev_device *hiddev, PointingDeviceDescriptor &desc);

    void processMatching(PointingDeviceData *pdd, SystemPointingDevice *device);

    void checkFoundDevice(udev_device *device);
    void checkLostDevice(udev_device *device);

    void unSeizeDevice(linuxPointingDeviceData *data);
    virtual void removePointingDevice(SystemPointingDevice *device) override;

    linuxPointingDeviceManager();
    ~linuxPointingDeviceManager();
  };

}

#endif
