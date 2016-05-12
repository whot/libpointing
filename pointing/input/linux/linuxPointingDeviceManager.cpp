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

#include <X11/Xatom.h>
#include <X11/extensions/XInput.h>

#include <iostream>
#include <stdexcept>

using namespace std;

namespace pointing {

#define MAX(X, Y)           (((X) > (Y)) ? (X) : (Y))

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


    XDeviceInfo*
    find_device_info(Display	*display,
                     char		*name,
                     Bool		only_extended)
    {
      XDeviceInfo	*devices;
      XDeviceInfo *found = NULL;
      int		loop;
      int		num_devices;
      int		len = strlen(name);
      Bool	is_id = True;
      XID		id = (XID)-1;

      for(loop=0; loop<len; loop++) {
        if (!isdigit(name[loop])) {
          is_id = False;
          break;
        }
      }

      if (is_id) {
        id = atoi(name);
      }

      devices = XListInputDevices(display, &num_devices);

      for(loop=0; loop<num_devices; loop++) {
        if ((!only_extended || (devices[loop].use >= IsXExtensionDevice)) &&
            ((!is_id && strcmp(devices[loop].name, name) == 0) ||
             (is_id && devices[loop].id == id))) {
          if (found) {
            fprintf(stderr,
                    "Warning: There are multiple devices named '%s'.\n"
                    "To ensure the correct one is selected, please use "
                    "the device ID instead.\n\n", name);
            return NULL;
          } else {
            found = &devices[loop];
          }
        }
      }
      return found;
    }

    void linuxPointingDeviceManager::enableDevice(bool value, std::string fullName)
    {
      Display	*dpy = XOpenDisplay(0);
      XDeviceInfo *info = find_device_info(dpy, (char *)fullName.c_str(), False);
      if (!info)
      {
        fprintf(stderr, "unable to find the device\n");
        return;
      }

      XDevice *dev = XOpenDevice(dpy, info->id);
      if (!dev)
      {
        fprintf(stderr, "unable to open the device\n");
        return;
      }

      Atom prop = XInternAtom(dpy, "Device Enabled", False);

      unsigned char data = value;

      XChangeDeviceProperty(dpy, dev, prop, XA_INTEGER, 8, PropModeReplace,
                            &data, 1);
      XCloseDevice(dpy, dev);
      XCloseDisplay(dpy);
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

      PointingDeviceData *pdd = (PointingDeviceData *)context;
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
          if (pdd->desc.devURI == device->uri)
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
      if (device->seize)
        enableDevice(false, device->vendor + " " + device->product);
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

      if (debugLevel > 0)
      {
        bool match = pdd->pointingList.size();
        std::cout << (match ? "+ " : "  ") << pdd->desc.devURI
             << " [" << std::hex << "vend:0x" << pdd->desc.vendorID
             << ", prod:0x" << pdd->desc.productID
             << std::dec << " - " << pdd->desc.vendor
             << " " << pdd->desc.product << "]" << std::endl;
      }

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
      devMap_t::iterator it = devMap.find(devnode);
      if (it != devMap.end())
      {
        PointingDeviceData *pdd = it->second;
        if (pthread_cancel(pdd->thread) < 0)
          perror("linuxPointingDeviceManager::checkLostDevice");
        removeDevice(pdd->desc);
        close(pdd->devID);
        for (PointingList::iterator i = pdd->pointingList.begin(); i != pdd->pointingList.end(); i++)
        {
          linuxPointingDevice *device = *i;
          if (device->seize)
            enableDevice(true, device->vendor + " " + device->product);
          device->active = false;
          candidates.push_back(device);
        }
        delete pdd;
        devMap.erase(it);
      }
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

    void linuxPointingDeviceManager::addPointingDevice(linuxPointingDevice *device)
    {
      debugLevel = MAX(debugLevel, device->debugLevel);
      candidates.push_back(device);
      matchCandidates();
    }

    void linuxPointingDeviceManager::removePointingDevice(linuxPointingDevice *device)
    {
      URI uri = device->uri;
      for(devMap_t::iterator it = devMap.begin(); it != devMap.end(); it++)
      {
        PointingDeviceData *pdd = it->second;
        if (pdd->desc.devURI == uri)
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
      // TODO cancel all threads
      udev_monitor_unref(monitor);
      udev_unref(udev);
    }
}
