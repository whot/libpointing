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
#include <string>
#include <map>
#include <list>
#include <pointing/utils/HIDReportParser.h>

// TODO remove includes

namespace pointing {

  /**
   * @brief The linuxPointingDeviceManager class is the platform-specific
   * subclass of the PointingDeviceManager class
   */
  class linuxPointingDeviceManager : public PointingDeviceManager
  {
    friend class PointingDeviceManager;
    friend class linuxPointingDevice;

    typedef std::list<linuxPointingDevice *> PointingList;

    struct PointingDeviceData
    {
      PointingDeviceDescriptor desc;
      PointingList pointingList;
      HIDReportParser parser;
      int devID;
      int reportLength;
    };

    PointingList candidates;
    int debugLevel;

    typedef std::map<std::string, PointingDeviceData *> devMap_t;

    devMap_t devMap;

    struct udev *udev ;
    struct udev_monitor *monitor ;

    pthread_t thread ;

    /**
     * @brief This static function works in another thread.
     * It queries for added or removed devices.
     */
    static void *eventloop(void *self) ;

    void monitor_readable();
    void hid_readable(PointingDeviceData *pdd);

    int readHIDDescriptor(int devID, HIDReportParser *parser);
    void fillDescriptorInfo(struct udev_device *hiddev, struct udev_device *usbdev, PointingDeviceDescriptor &desc);

    void processMatching(PointingDeviceData *pdd, linuxPointingDevice *device);
    void convertAnyCandidates();
    void matchCandidates();

    void checkFoundDevice(struct udev_device *device) ;
    void checkLostDevice(struct udev_device *device) ;

    void addPointingDevice(linuxPointingDevice *device);
    void removePointingDevice(linuxPointingDevice *device);

    linuxPointingDeviceManager();
    ~linuxPointingDeviceManager();
  };

}

#endif
