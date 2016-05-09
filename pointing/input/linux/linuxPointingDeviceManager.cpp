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

#define MAX(X, Y)           (((X) > (Y)) ? (X) : (Y))
  
    string uriStringFromDevice(struct udev_device *hiddev)
    {
      const char *devnode = udev_device_get_devnode(hiddev);
      URI devUri;
      devUri.scheme = "hidraw";
      devUri.path = devnode;
      return devUri.asString();
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

    void *linuxPointingDeviceManager::eventloop(void *context)
    {
      pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL) ;
      pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL) ;

      linuxPointingDeviceManager *self = (linuxPointingDeviceManager*)context ;

      udev_monitor_enable_receiving(self->monitor) ;
      int monfd = udev_monitor_get_fd(self->monitor) ;
      while (true)
      {
        if (checkDev(monfd))
          self->monitor_readable();
        for(devMap_t::iterator it = self->devMap.begin(); it != self->devMap.end(); it++)
        {
          if (checkDev(it->second->devID))
            self->hid_readable(it->second);
        }
      }
      return 0 ;
    }

    linuxPointingDeviceManager::linuxPointingDeviceManager()
    {
      udev = udev_new();
      if (!udev)
        throw runtime_error("linuxPointingDeviceManager: udev_new failed");

      struct udev_enumerate *enumerate = udev_enumerate_new(udev);
      udev_enumerate_add_match_subsystem(enumerate, "hidraw");
      udev_enumerate_scan_devices(enumerate);
      struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
      struct udev_list_entry *dev_list_entry;
      udev_list_entry_foreach(dev_list_entry, devices) {
        const char *path = udev_list_entry_get_name(dev_list_entry);
        struct udev_device *dev = udev_device_new_from_syspath(udev, path);
        //cerr << uriStringFromDevice(dev) << endl;
        checkFoundDevice(dev);
        udev_device_unref(dev);
      }
      udev_enumerate_unref(enumerate);

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
      desc.devURI = URI(uriStringFromDevice(hiddev));
      desc.vendor = sysattr2string(usbdev, "product", "????");
      desc.product = sysattr2string(usbdev, "manufacturer", "????");
      desc.vendorID = sysattr2int(usbdev, "idVendor");
      desc.productID = sysattr2int(usbdev, "idProduct");
    }

    void linuxPointingDeviceManager::convertAnyCandidates()
    {
      for (PointingList::iterator it = candidates.begin(); it != candidates.end(); it++)
      {
        linuxPointingDevice *device = *it;
        if (!device->anyURI.asString().empty())
          device->uri = anyToSpecific(device->anyURI);
      }
    }

    void linuxPointingDeviceManager::matchCandidates()
    {
      convertAnyCandidates();
      for(devMap_t::iterator it = devMap.begin(); it != devMap.end(); it++)
      {
        PointingDeviceData *pdd = it->second;

        PointingList::iterator i = candidates.begin();
        while (i != candidates.end())
        {
          linuxPointingDevice *device = *i;
          // Found matching device
          // Move it from candidates to devMap
          if (pdd->desc.devURI == device->uri.asString())
          {
            candidates.erase(i++);
            processMatching(pdd, device);
          }
          else
            i++;
        }
      }
    }

    void linuxPointingDeviceManager::processMatching(PointingDeviceData *pdd, linuxPointingDevice *device)
    {
      pdd->pointingList.push_back(device);
      device->active = true;
      device->productID = pdd->desc.productID;
      device->vendorID = pdd->desc.vendorID;
      device->vendor = pdd->desc.vendor;
      device->product = pdd->desc.product;
    }

    int linuxPointingDeviceManager::readHIDDescriptor(int devID, HIDReportParser *parser)
    {
      int descSize = 0;
      int res = ioctl(devID, HIDIOCGRDESCSIZE, &descSize);
      if (res < 0) {
        std::cerr << "linuxPointingDevice::checkFoundDevice: unable to open HID device" << std::endl ;
        return 0;
      }
      if (debugLevel > 0)
        std::cerr << "  descriptor size: " << descSize << std::endl ;
      struct hidraw_report_descriptor descriptor ;
      descriptor.size = descSize ;
      res = ioctl(devID, HIDIOCGRDESC, &descriptor) ;
      if (!parser->setDescriptor(descriptor.value, descSize))
        std::cerr << "linuxPointingDevice::checkFoundDevice: unable to parse the HID report descriptor" << std::endl;
      if (res < 0) {
        perror("linuxPointingDevice::checkFoundDevice") ;
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
      PointingDeviceData *pdd = new PointingDeviceData;
      devMap[devnode] = pdd;
      pdd->devID = devID;
      fillDescriptorInfo(hiddev, usbdev, pdd->desc);
      addDevice(pdd->desc);
      matchCandidates();

      pdd->reportLength = readHIDDescriptor(devID, &pdd->parser);
      //if (self->debugLevel > 0)
      //  printDevice(devRef, pdd->pointingList.size());
    }

    void linuxPointingDeviceManager::checkLostDevice(struct udev_device *hiddev)
    {
      const char *devnode = udev_device_get_devnode(hiddev);
      devMap_t::iterator it = devMap.find(devnode);
      if (it != devMap.end())
      {
        PointingDeviceData *pdd = it->second;
        removeDevice(pdd->desc);
        close(pdd->devID);
        for (PointingList::iterator it = pdd->pointingList.begin(); it != pdd->pointingList.end(); it++)
        {
          linuxPointingDevice *device = *it;
          device->active = false;
          candidates.push_back(device);
        }
        delete pdd;
        devMap.erase(it);
      }
      matchCandidates();
    }

    void linuxPointingDeviceManager::addPointingDevice(linuxPointingDevice *device)
    {
      debugLevel = MAX(debugLevel, device->debugLevel);
      candidates.push_back(device);
      matchCandidates();
    }

    void linuxPointingDeviceManager::hid_readable(PointingDeviceData *pdd)
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
        for (PointingList::iterator it = pdd->pointingList.begin(); it != pdd->pointingList.end(); it++)
        {
          linuxPointingDevice *device = *it;
          device->registerTimestamp(now);
          if (device->callback)
            device->callback(device->callback_context, now, dx, dy, buttons);
        }
      }
    }

    void linuxPointingDeviceManager::removePointingDevice(linuxPointingDevice *device)
    {
      URI uri = device->getURI();
      for(devMap_t::iterator it = devMap.begin(); it != devMap.end(); it++)
      {
        PointingDeviceData *pdd = it->second;
        if (pdd->desc.devURI == uri.asString())
        {
          pdd->pointingList.remove(device);
          break;
        }
      }
      candidates.remove(device);
    }

    linuxPointingDeviceManager::~linuxPointingDeviceManager()
    {
      if (pthread_cancel(thread) < 0)
        perror("linuxPointingDeviceManager::~linuxPointingDeviceManager");
      udev_monitor_unref(monitor);
      udev_unref(udev);
    }
}
