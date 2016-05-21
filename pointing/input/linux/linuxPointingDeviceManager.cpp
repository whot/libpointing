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
// 2. Any not working with vendor = 0
// 3. Check device capabilities
// 4. Try to change the device mode
// 5. Rate from touchpad

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
      if (checkDev(pdd->fd))
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
    desc.devURI = uriFromDevice(hiddev);
    udev_device *dev = udev_device_get_parent(hiddev);
    if (!dev)
      return;
    desc.vendorID = sysattr2int(dev, "id/vendor");
    desc.productID = sysattr2int(dev, "id/product");
    desc.product = sysattr2string(dev, "name", "???");
  }

  void linuxPointingDeviceManager::processMatching(PointingDeviceData *data, SystemPointingDevice *device)
  {
    linuxPointingDevice *dev = static_cast<linuxPointingDevice *>(device);
    linuxPointingDeviceData *pdd = static_cast<linuxPointingDeviceData *>(data);

    if (dev->seize && !pdd->seizeCount++)
    {
      if (ioctl(pdd->fd, EVIOCGRAB, 1) != 0)
        std::cerr << "linuxPointingDeviceManager::processMatching: could not seize the device" << std::endl;
    }
    udev_device *par = udev_device_get_parent_with_subsystem_devtype(pdd->evDev, "usb", "usb_interface");
    int ms = sysattr2int(par, "ep_81/bInterval");
    if (ms)
      dev->hz = 1000.0 / ms;
  }

  void linuxPointingDeviceManager::checkFoundDevice(udev_device *hiddev)
  {
    const char *name = udev_device_get_sysname(hiddev);
    if (!name)
      return;
    bool matchMouse = strncmp(name, "mouse", 5) == 0;
    bool matchMice = strcmp(name, "mice") == 0;
    if (!matchMouse && !matchMice)
      return;
    udevDebugDevice(hiddev, std::cerr);
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

    pdd->fd = open(devnode, O_RDONLY);
    // Non blocking
    int flags = fcntl(pdd->fd, F_GETFL, 0);
    fcntl(pdd->fd, F_SETFL, flags | O_NONBLOCK);
    if (pdd->fd == -1) {
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
      if (pdd->fd > -1)
        close(pdd->fd);
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
    if (pdd && !pdd->seizeCount)
    {
      unSeizeDevice(pdd);
    }
    PointingDeviceManager::removePointingDevice(device);
  }

  void linuxPointingDeviceManager::unSeizeDevice(linuxPointingDeviceData *pdd)
  {
    if (pdd->fd > 0)
    {
      ioctl(pdd->fd, EVIOCGRAB, 0);
      pdd->seizeCount = 0;
    }
  }

  void linuxPointingDeviceManager::readable(linuxPointingDeviceData *pdd)
  {
    TimeStamp::inttime now = TimeStamp::createAsInt();
    input_event ie;

    int dx = 0, dy = 0;

    while (read(pdd->fd, &ie, sizeof(input_event)) > 0)
    {
      if (ie.type == EV_REL)
      {
        if (ie.code == REL_X)
          dx = ie.value;
        else if (ie.code == REL_Y)
          dy = ie.value;
      }
      else if (ie.type == EV_KEY)
      {
        if (ie.code == BTN_LEFT)
          pdd->buttons = ie.value ? (pdd->buttons | (1 << 0)) : (pdd->buttons & ~(1 << 0));
        else if (ie.code == BTN_RIGHT)
          pdd->buttons = ie.value ? (pdd->buttons | (1 << 1)) : (pdd->buttons & ~(1 << 1));
        else if (ie.code == BTN_MIDDLE)
          pdd->buttons = ie.value ? (pdd->buttons | (1 << 2)) : (pdd->buttons & ~(1 << 2));
      }
    }

    for (SystemPointingDevice *device : pdd->pointingList)
    {
      linuxPointingDevice *dev = static_cast<linuxPointingDevice *>(device);
      dev->registerTimestamp(now);
      if (dev->callback)
        dev->callback(dev->callback_context, now, dx, dy, pdd->buttons);
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
