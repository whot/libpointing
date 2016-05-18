/* -*- mode: c++ -*-
 *
 * pointing/input/linux/linuxPointingDeviceManager.cpp --
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

#include <pointing/input/linux/linuxPointingDeviceManager.h>

#include <pointing/utils/Base64.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pointing/utils/URI.h>

#include <linux/input.h>
#include <linux/hidraw.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include <iostream>
#include <stdexcept>

using namespace std;

namespace pointing {

#define EVENT_DEV "/dev/input/event"

  URI uriFromDevice(struct udev_device *hiddev)
  {
    const char *devnode = udev_device_get_devnode(hiddev);
    URI devUri;
    devUri.scheme = "hidraw";
    devUri.path = devnode;
    return devUri;
  }

  static inline string sysattr2string(struct udev_device *dev, const char *key, const char *defval=0)
  {
    const char *value = udev_device_get_sysattr_value(dev, key);
    return value ? value : (defval ? defval : string());
  }

  static inline int sysattr2int(struct udev_device *dev, const char *key, int defval=0)
  {
    const char *value = udev_device_get_sysattr_value(dev, key);
    return value ? strtol(value, NULL, 16) : defval;
  }

  static inline void udevDebugDevice(struct udev_device *dev, std::ostream& out)
  {
     out << std::endl ;
     const char *s = udev_device_get_devpath(dev) ;
     out << "devpath: " << (s?s:"NULL") << std::endl ;
     s = udev_device_get_subsystem(dev) ;
     out << "  subsystem: " << (s?s:"NULL") << std::endl ;
     s = udev_device_get_devtype(dev) ;
     out << "  devtype: " << (s?s:"NULL") << std::endl ;
     s = udev_device_get_syspath(dev) ;
     out << "  syspath: " << (s?s:"NULL") << std::endl ;
     s = udev_device_get_sysname(dev) ;
     out << "  sysname: " << (s?s:"NULL") << std::endl ;
     s = udev_device_get_sysnum(dev) ;
     out << "  sysnum: " << (s?s:"NULL") << std::endl ;
     s = udev_device_get_devnode(dev) ;
     out << "  devnode: " << (s?s:"NULL") << std::endl ;
     s = udev_device_get_driver(dev) ;
     out << "  driver: " << (s?s:"NULL") << std::endl ;
     out << std::endl ;
  }

  bool checkDev(int devID)
  {
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(devID, &rfds);
    int nbready = select(devID + 1, &rfds, NULL, NULL, 0);
    pthread_testcancel();
    if (nbready == -1)
      perror("linuxPointingDevice::eventloop");
    return FD_ISSET(devID, &rfds);
  }

  struct udev_device* findEventDevNode(struct udev* udev, struct udev_device* dev)
  {
    // This function finds the associated event devnode
    // For a given hidraw devnode
    for (int i = 0; i < 5; i++)
    {
      dev = udev_device_get_parent(dev);
      if (!dev)
        return NULL;

      struct udev_enumerate *enumerate = udev_enumerate_new(udev);

      udev_enumerate_add_match_parent(enumerate, dev);
      udev_enumerate_scan_devices(enumerate);

      struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
      struct udev_list_entry *entry;

      udev_list_entry_foreach(entry, devices)
      {
        const char *path = udev_list_entry_get_name(entry);
        struct udev_device* child = udev_device_new_from_syspath(udev, path);
        const char *devnode = udev_device_get_devnode(child);
        if (devnode && strncmp(devnode, EVENT_DEV, strlen(EVENT_DEV)) == 0)
        {
          // Found corresponding event devnode
          udev_enumerate_unref(enumerate);
          return child;
        }
      }
      udev_enumerate_unref(enumerate);
    }
    return NULL;
  }

  void *linuxPointingDeviceManager::eventloop(void *context)
  {
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL) ;
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL) ;

    linuxPointingDeviceManager *self = (linuxPointingDeviceManager*)context ;

    struct udev_enumerate *enumerate = udev_enumerate_new(self->udev);
    udev_enumerate_add_match_subsystem(enumerate, "hidraw");
    udev_enumerate_scan_devices(enumerate);

    struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
    struct udev_list_entry *dev_list_entry;
    udev_list_entry_foreach(dev_list_entry, devices) {
      const char *path = udev_list_entry_get_name(dev_list_entry);
      struct udev_device *dev = udev_device_new_from_syspath(self->udev, path);
      self->checkFoundDevice(dev);
      udev_device_unref(dev);
    }
    udev_enumerate_unref(enumerate);

    udev_monitor_enable_receiving(self->monitor) ;
    int monfd = udev_monitor_get_fd(self->monitor) ;
    while (true)
    {
      if (checkDev(monfd))
        self->monitor_readable();
    }
    return 0 ;
  }

  void *linuxPointingDeviceManager::checkReports(void *context)
  {
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL) ;
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL) ;

    linuxPointingDeviceData *pdd = (linuxPointingDeviceData *)context;
    linuxPointingDeviceManager *self = (linuxPointingDeviceManager *)PointingDeviceManager::get();

    while (true)
    {
      if (checkDev(pdd->devID))
        self->hid_readable(pdd);
    }
    return 0 ;
  }

  linuxPointingDeviceManager::linuxPointingDeviceManager()
  {
    udev = udev_new();
    if (!udev)
      throw runtime_error("linuxPointingDeviceManager: udev_new failed");

    monitor = udev_monitor_new_from_netlink(udev, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(monitor, "hidraw", NULL);

    int ret = pthread_create(&thread, NULL, eventloop, (void*)this);
    if (ret < 0)
    {
      perror("linuxPointingDeviceManager::linuxPointingDeviceManager") ;
      throw runtime_error("linuxPointingDeviceManager: pthread_create failed") ;
    }
  }

  void linuxPointingDeviceManager::monitor_readable(void)
  {
      struct udev_device *dev = udev_monitor_receive_device(monitor) ;
      if (dev)
      {
        const char *action = udev_device_get_action(dev) ;
        if (!strcmp(action,"add")) {
          checkFoundDevice(dev) ;
        } else if (!strcmp(action,"remove")) {
          checkLostDevice(dev) ;
        }
        udev_device_unref(dev) ;
      }
  }

  void linuxPointingDeviceManager::fillDescriptorInfo(struct udev_device *hiddev, struct udev_device *usbdev, PointingDeviceDescriptor &desc)
  {
    desc.devURI = uriFromDevice(hiddev);
    desc.vendor = sysattr2string(usbdev, "manufacturer", "???");
    desc.product = sysattr2string(usbdev, "product", "???");
    desc.vendorID = sysattr2int(usbdev, "idVendor");
    desc.productID = sysattr2int(usbdev, "idProduct");
  }

  void linuxPointingDeviceManager::processMatching(PointingDeviceData *data, SystemPointingDevice *device)
  {
    linuxPointingDevice *dev = static_cast<linuxPointingDevice *>(device);
    linuxPointingDeviceData *pdd = static_cast<linuxPointingDeviceData *>(data);

    if (dev->seize)
    {
      if (pdd->evDevId < 0)
      {
        pdd->evDevId = open(pdd->evDevNode.c_str(), O_RDONLY);
        if (pdd->evDevId == -1)
          std::cerr << "linuxPointingDeviceManager::processMatching: failed to open evDevNode" << std::endl;
        int result = ioctl(pdd->evDevId, EVIOCGRAB, 1);
        if (result != 0)
          std::cerr << "linuxPointingDeviceManager::processMatching: could not seize the device" << std::endl;
        if (dev->debugLevel > 1)
        	std::cerr << "    " << dev->uri << " corresponds to " << pdd->evDevNode << std::endl;
      }
      pdd->seizeCount++;
    }
  }

  int linuxPointingDeviceManager::readHIDDescriptor(int devID, HIDReportParser *parser)
  {
    int descSize = 0;
    int res = ioctl(devID, HIDIOCGRDESCSIZE, &descSize);
    if (res < 0) {
      std::cerr << "linuxPointingDeviceManager::checkFoundDevice: unable to open HID device" << std::endl ;
      return 0;
    }
    if (debugLevel > 0)
      std::cerr << "  descriptor size: " << descSize << std::endl ;
    struct hidraw_report_descriptor descriptor ;
    descriptor.size = descSize ;
    res = ioctl(devID, HIDIOCGRDESC, &descriptor) ;
    if (!parser->setDescriptor(descriptor.value, descSize))
      std::cerr << "linuxPointingDeviceManager::readHIDDescriptor: unable to parse the HID report descriptor" << std::endl;
    if (res < 0) {
      perror("linuxPointingDeviceManager::checkFoundDevice") ;
      return 0;
    } else {
      if (debugLevel > 1) {
        std::cerr << "  descriptor (" << descriptor.size << " bytes): " ;
        std::string reportstring ;
        reportstring.assign((const char *)descriptor.value, descriptor.size) ;
        std::cerr << Base64::encode(reportstring) << std::endl ;
      }
    }
    return parser->getReportLength();
  }

  static inline std::string
  bustype2string(int bustype) {
    const char *names[] = {"???", "PCI", "ISAPNP", "USB", "HIL", "BLUETOOTH", "VIRTUAL"} ;
    if (bustype<0 || bustype>BUS_VIRTUAL) return names[0] ;
    return names[bustype] ;
  }

  void linuxPointingDeviceManager::checkFoundDevice(struct udev_device *hiddev)
  {
    struct udev_device *usbdev = udev_device_get_parent_with_subsystem_devtype(hiddev, "usb", "usb_device") ;
    if (!usbdev) return ;

    const char *devnode = udev_device_get_devnode(hiddev);
    int devID = open(devnode, O_RDONLY);
    if (devID == -1) {
      std::cerr << "linuxPointingDeviceManager::checkFoundDevice: unable to open HID device" << std::endl ;
      return ;
    }
    linuxPointingDeviceData *pdd = new linuxPointingDeviceData;
    pdd->devID = devID;
    fillDescriptorInfo(hiddev, usbdev, pdd->desc);
    struct udev_device *eventDev = findEventDevNode(udev, hiddev);
    if (eventDev)
      pdd->evDevNode = udev_device_get_devnode(eventDev);
    registerDevice(devnode, pdd);

    pdd->reportLength = readHIDDescriptor(devID, &pdd->parser);

    int ret = pthread_create(&pdd->thread, NULL, checkReports, pdd);
    if (ret < 0)
    {
      perror("linuxPointingDeviceManager::checkFoundDevice") ;
      throw runtime_error("linuxPointingDeviceManager: pthread_create failed") ;
    }
  }

  void linuxPointingDeviceManager::checkLostDevice(struct udev_device *hiddev)
  {
    const char *devnode = udev_device_get_devnode(hiddev);
    auto it = devMap.find(devnode);
    if (it != devMap.end())
    {
      linuxPointingDeviceData *pdd = static_cast<linuxPointingDeviceData *>(it->second);
      if (pthread_cancel(pdd->thread) < 0)
        perror("linuxPointingDeviceManager::checkLostDevice");
      close(pdd->devID);
      unSeizeDevice(pdd);
    }
    unregisterDevice(devnode);
  }

  void linuxPointingDeviceManager::removePointingDevice(SystemPointingDevice *device)
  {
    linuxPointingDeviceData *pdd = static_cast<linuxPointingDeviceData *>(findDataForDevice(device));
    pdd->seizeCount--;
    if (!pdd->seizeCount)
      unSeizeDevice(pdd);
    PointingDeviceManager::removePointingDevice(device);
  }

  void linuxPointingDeviceManager::unSeizeDevice(linuxPointingDeviceData *pdd)
  {
    if (pdd->evDevId > 0)
    {
      ioctl(pdd->evDevId, EVIOCGRAB, 0);
      close(pdd->evDevId);
      pdd->evDevId = -1;
      pdd->evDevNode = "";
      pdd->seizeCount = 0;
    }
  }

  void linuxPointingDeviceManager::hid_readable(linuxPointingDeviceData *pdd)
  {
    TimeStamp::inttime now = TimeStamp::createAsInt() ;
    unsigned char *report = new unsigned char[pdd->reportLength];
    int32_t length = read(pdd->devID, report, pdd->reportLength);
    pdd->parser.setReport(report);
    delete report;

    if (length > 0)
    {
      int dx=0, dy=0, buttons=0;
      pdd->parser.getReportData(&dx, &dy, &buttons);
      for (SystemPointingDevice *device : pdd->pointingList)
      {
        linuxPointingDevice *dev = static_cast<linuxPointingDevice *>(device);
        dev->registerTimestamp(now);
        if (dev->callback)
          dev->callback(dev->callback_context, now, dx, dy, buttons);
      }
    }
  }

  linuxPointingDeviceManager::~linuxPointingDeviceManager()
  {
    if (pthread_cancel(thread) < 0)
      perror("linuxPointingDeviceManager::~linuxPointingDeviceManager");
    // TODO cancel all threads
    udev_monitor_unref(monitor);
    udev_unref(udev);
  }
}
