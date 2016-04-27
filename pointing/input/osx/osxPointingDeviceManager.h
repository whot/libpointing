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
#include <list>
#include <pointing/input/PointingDeviceManager.h>
#include <IOKit/hid/IOHIDManager.h>
#include <pointing/input/osx/osxPointingDevice.h>
#include <pointing/utils/HIDReportParser.h>

#include <iomanip>

namespace pointing
{
/**
   * @brief The osxPointingDeviceManager class is the platform-specific
   * subclass of the PointingDeviceManager class.
   *
   * There are no public members of this class, because all the functions are called by
   * either its parent or osxPointingDevice which are friends of this class.
   */
  class osxPointingDeviceManager : public PointingDeviceManager
  {
    friend class PointingDeviceManager;
    friend class osxPointingDevice;

    typedef std::list<osxPointingDevice *> PointingList;

    struct PointingDeviceData
    {
      PointingDeviceDescriptor desc;
      PointingList pointingList;
      HIDReportParser parser;
      uint8_t report[64];
      IOHIDDeviceRef devRef;
    };

    PointingList candidates;
    int debugLevel;

    typedef std::map<IOHIDDeviceRef, PointingDeviceData *> devMap_t;

    void convertAnyCandidates();
    void matchCandidates();
    void processMatching(PointingDeviceData *pdd, osxPointingDevice *device);

    // Map is needed because we cannot find all the information about removed device
    devMap_t devMap;

    IOHIDManagerRef manager;
    static void AddDevice(void *context, IOReturn /*result*/, void *sender, IOHIDDeviceRef devRef);
    static void RemoveDevice(void *context, IOReturn /*result*/, void *sender, IOHIDDeviceRef devRef);

    osxPointingDeviceManager();
    ~osxPointingDeviceManager() {}

    void addPointingDevice(osxPointingDevice *device);
    void removePointingDevice(osxPointingDevice *device);

    static void hidReportCallback(void *context, IOReturn result, void *sender,
                  IOHIDReportType type, uint32_t reportID,
                  uint8_t *report, CFIndex reportLength) ;
  };
}


#endif
