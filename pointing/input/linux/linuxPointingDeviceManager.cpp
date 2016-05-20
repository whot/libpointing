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

#include <X11/Xatom.h>
#include <X11/extensions/XInput.h>

#include <pointing/utils/URI.h>

#include <linux/input.h>
#include <linux/hidraw.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include <iostream>
#include <stdexcept>

// TODO:
// 1. Unblock the touchpad
// 2. Determine frequency
// 3. Look EVIOCGREP and others

using namespace std;

namespace pointing {

#define EVENT_DEV "/dev/input/event"

  URI uriFromDevice(udev_device *hiddev)
  {
    const char *devnode = udev_device_get_devnode(hiddev);
    URI devUri;
    devUri.scheme = "input";
    devUri.path = devnode;
    return devUri;
  }

  static inline string sysattr2string(udev_device *dev, const char *key, const char *defval=0)
  {
    const char *value = udev_device_get_sysattr_value(dev, key);
    return value ? value : (defval ? defval : string());
  }

  static inline int sysattr2int(udev_device *dev, const char *key, int defval=0)
  {
    const char *value = udev_device_get_sysattr_value(dev, key);
    return value ? strtol(value, NULL, 16) : defval;
  }

  static inline void udevDebugDevice(udev_device *dev, std::ostream& out)
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

  udev_device* findEvDev(udev* udev, udev_device* dev)
  {
    // This function finds the associated event devnode
    // For a given "mouse" devnode
    dev = udev_device_get_parent(dev);
    if (!dev)
      return NULL;

    udev_enumerate *enumerate = udev_enumerate_new(udev);

    udev_enumerate_add_match_parent(enumerate, dev);
    udev_enumerate_scan_devices(enumerate);

    udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
    udev_list_entry *entry;

    udev_list_entry_foreach(entry, devices)
    {
      const char *path = udev_list_entry_get_name(entry);
      udev_device* child = udev_device_new_from_syspath(udev, path);
      const char *devnode = udev_device_get_devnode(child);
      if (devnode && strncmp(devnode, EVENT_DEV, strlen(EVENT_DEV)) == 0)
      {
        // Found corresponding event devnode
        // Need to unref it at the end
        return child;
      }
      udev_device_unref(child);
    }
    udev_enumerate_unref(enumerate);
    return NULL;
  }

  void *linuxPointingDeviceManager::eventloop(void *context)
  {
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL) ;
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL) ;

    linuxPointingDeviceManager *self = (linuxPointingDeviceManager*)context ;

    udev_enumerate *enumerate = udev_enumerate_new(self->udev);
    udev_enumerate_add_match_subsystem(enumerate, "input");
    udev_enumerate_scan_devices(enumerate);

    udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
    udev_list_entry *dev_list_entry;
    udev_list_entry_foreach(dev_list_entry, devices) {
      const char *path = udev_list_entry_get_name(dev_list_entry);
      udev_device *dev = udev_device_new_from_syspath(self->udev, path);
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
        self->readable(pdd);
    }
    return 0 ;
  }

  linuxPointingDeviceManager::linuxPointingDeviceManager()
  {
    udev = udev_new();
    if (!udev)
      throw runtime_error("linuxPointingDeviceManager: udev_new failed");

    monitor = udev_monitor_new_from_netlink(udev, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(monitor, "input", NULL);

    int ret = pthread_create(&thread, NULL, eventloop, (void*)this);
    if (ret < 0)
    {
      perror("linuxPointingDeviceManager::linuxPointingDeviceManager") ;
      throw runtime_error("linuxPointingDeviceManager: pthread_create failed") ;
    }
  }

  void linuxPointingDeviceManager::monitor_readable(void)
  {
      udev_device *dev = udev_monitor_receive_device(monitor) ;
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

  void linuxPointingDeviceManager::fillExternalDescInfo(udev_device *hiddev, udev_device *usbdev, PointingDeviceDescriptor &desc)
  {
    desc.devURI = uriFromDevice(hiddev);
    desc.vendor = sysattr2string(usbdev, "manufacturer", "???");
    desc.product = sysattr2string(usbdev, "product", "???");
    desc.vendorID = sysattr2int(usbdev, "idVendor");
    desc.productID = sysattr2int(usbdev, "idProduct");
  }

  void linuxPointingDeviceManager::fillEmbeddedDescInfo(udev_device *hiddev, PointingDeviceDescriptor &desc)
  {
    // TODO
    desc.devURI = uriFromDevice(hiddev);
    //desc.vendor = sysattr2string(usbdev, "manufacturer", "???");
    //desc.product = sysattr2string(usbdev, "product", "???");
    //desc.vendorID = sysattr2int(usbdev, "idVendor");
    //desc.productID = sysattr2int(usbdev, "idProduct");
  }

  void linuxPointingDeviceManager::processMatching(PointingDeviceData *data, SystemPointingDevice *device)
  {
    linuxPointingDevice *dev = static_cast<linuxPointingDevice *>(device);
    linuxPointingDeviceData *pdd = static_cast<linuxPointingDeviceData *>(data);

    if (dev->seize)
    {
      int result = ioctl(pdd->devID, EVIOCGRAB, 1);
      if (result != 0)
        std::cerr << "linuxPointingDeviceManager::processMatching: could not seize the device" << std::endl;
      else
        pdd->seizeCount++;
    }
  }

  /*
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
  */


  void linuxPointingDeviceManager::checkFoundDevice(udev_device *hiddev)
  {
    const char *name = udev_device_get_sysname(hiddev);
    if (!name)
      return;
    bool matchMouse = strncmp(name, "mouse", 5) == 0;
    bool matchMice = strcmp(name, "mice") == 0;
    if (!matchMouse && !matchMice)
      return;
    //udevDebugDevice(hiddev, std::cerr);
    linuxPointingDeviceData *pdd = new linuxPointingDeviceData;
    const char *devnode = NULL;
    if (matchMouse)
    {
      udev_device *usbdev = udev_device_get_parent_with_subsystem_devtype(hiddev, "usb", "usb_device");
      // If there is no usbdev, most probably the embedded touchpad was found
      if (usbdev)
      {
        fillExternalDescInfo(hiddev, usbdev, pdd->desc);
      }
      else
      {
        fillEmbeddedDescInfo(hiddev, pdd->desc);
      }
      pdd->evDev = findEvDev(udev, hiddev);
      devnode = udev_device_get_devnode(pdd->evDev);
    }
    else // matchMice
    {
      pdd->desc.devURI = uriFromDevice(hiddev);
      pdd->desc.product = "Mice";
      pdd->desc.vendor = "Virtual";
      devnode = udev_device_get_devnode(hiddev);
    }

    pdd->devID = open(devnode, O_RDONLY);
    if (pdd->devID == -1) {
      std::cerr << "linuxPointingDeviceManager::checkFoundDevice: unable to open " << devnode << std::endl;
      if (pdd->evDev)
          udev_device_unref(pdd->evDev);
      delete pdd;
      return;
    }

    registerDevice(devnode, pdd);

    int ret = pthread_create(&pdd->thread, NULL, checkReports, pdd);
    if (ret < 0)
    {
      perror("linuxPointingDeviceManager::checkFoundDevice");
      throw runtime_error("linuxPointingDeviceManager: pthread_create failed");
    }
  }

  void linuxPointingDeviceManager::checkLostDevice(udev_device *hiddev)
  {
    const char *devnode = udev_device_get_devnode(hiddev);
    if (!devnode)
      return;
    auto it = devMap.find(devnode);
    if (it != devMap.end())
    {
      linuxPointingDeviceData *pdd = static_cast<linuxPointingDeviceData *>(it->second);
      if (pthread_cancel(pdd->thread) < 0)
        perror("linuxPointingDeviceManager::checkLostDevice");
      unSeizeDevice(pdd);
      if (pdd->devID > -1)
        close(pdd->devID);
      if (pdd->evDev)
        udev_device_unref(pdd->evDev) ;
      unregisterDevice(devnode);
    }
  }

  void linuxPointingDeviceManager::removePointingDevice(SystemPointingDevice *device)
  {
    linuxPointingDeviceData *pdd = static_cast<linuxPointingDeviceData *>(findDataForDevice(device));
    linuxPointingDevice *dev = static_cast<linuxPointingDevice *>(device);
    if (dev->seize)
      pdd->seizeCount--;
    if (!pdd->seizeCount)
      unSeizeDevice(pdd);
    PointingDeviceManager::removePointingDevice(device);
  }

  void linuxPointingDeviceManager::unSeizeDevice(linuxPointingDeviceData *pdd)
  {
    if (pdd->devID > 0)
    {
      ioctl(pdd->devID, EVIOCGRAB, 0);
      pdd->seizeCount = 0;
    }
  }

  void linuxPointingDeviceManager::readable(linuxPointingDeviceData *pdd)
  {
    TimeStamp::inttime now = TimeStamp::createAsInt();
    input_event ie;
    int result = read(pdd->devID, &ie, sizeof(input_event));

    std::cerr << "Type: " << std::hex << ie.type << std::endl;
    std::cerr << "Code: " << std::hex << ie.code << std::endl;
    std::cerr << "Value: " << std::hex << ie.value << std::endl;

    if (result != -1)
    {
      unsigned char *ptr = (unsigned char*)&ie;

      int buttons = ptr[0] & 7; // 3 bits only
      int dx = (char)ptr[1];
      int dy = (char)ptr[2];

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
