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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <pointing/utils/URI.h>

#include <linux/hidraw.h>
#include <iostream>
#include <stdexcept>

using namespace std;

namespace pointing {
  
    string uriStringFromDevice(struct udev_device *hiddev)
    {
        const char *devnode = udev_device_get_devnode(hiddev);
        URI devUri;
        devUri.scheme = "hidraw" ;
        devUri.path = devnode ;
        return devUri.asString();
    }
    
    static inline string sysattr2string(struct udev_device *dev, const char *key, const char *defval=0)
    {
        const char *value = udev_device_get_sysattr_value(dev, key);
        return value ? value : (defval ? defval : string()) ;
    }

    static inline int sysattr2int(struct udev_device *dev, const char *key, int defval=0)
    {
        const char *value = udev_device_get_sysattr_value(dev, key);
        return value ? strtol(value, NULL, 16) : defval;
    }
    
    void *linuxPointingDeviceManager::eventloop(void *context)
    {
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL) ;
        pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL) ;

        linuxPointingDeviceManager *self = (linuxPointingDeviceManager*)context ;
       
        udev_monitor_enable_receiving(self->monitor) ;
        int monfd = udev_monitor_get_fd(self->monitor) ;
        while (true) {
          fd_set rfds;
          FD_ZERO(&rfds) ;
          FD_SET(monfd, &rfds) ;
          int nbready = select(monfd + 1, &rfds, NULL, NULL, 0) ;
          pthread_testcancel() ;
          if (nbready==-1)
	        perror("linuxPointingDeviceManager::eventloop") ;
          else {
	        if (FD_ISSET(monfd, &rfds)) self->monitor_readable() ;
          }
        }
        return 0 ;
    }

    linuxPointingDeviceManager::linuxPointingDeviceManager()
    {
        udev = udev_new() ;
        if (!udev)
          throw runtime_error("linuxPointingDeviceManager: udev_new failed") ;

        struct udev_enumerate *enumerate = udev_enumerate_new(udev) ;
        udev_enumerate_add_match_subsystem(enumerate, "hidraw") ;
        udev_enumerate_scan_devices(enumerate) ;
        struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate) ;
        struct udev_list_entry *dev_list_entry ;
        udev_list_entry_foreach(dev_list_entry, devices) {
          const char *path = udev_list_entry_get_name(dev_list_entry) ;
          struct udev_device *dev = udev_device_new_from_syspath(udev, path) ;
          //cerr << uriStringFromDevice(dev) << endl;
          checkFoundDevice(dev);
          udev_device_unref(dev);
        }
        udev_enumerate_unref(enumerate);
        
        monitor = udev_monitor_new_from_netlink(udev, "udev") ;
        udev_monitor_filter_add_match_subsystem_devtype(monitor, "hidraw", NULL) ;
        
        int ret = pthread_create(&thread, NULL, eventloop, (void*)this) ;
        if (ret < 0) {
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
  
    void linuxPointingDeviceManager::ConvertDevice(struct udev_device *hiddev, struct udev_device *usbdev, PointingDeviceDescriptor &desc)
    {
        string uri = uriStringFromDevice(hiddev);
        desc.devURI = uri;
        string vendor = sysattr2string(usbdev, "product", "????") ;
        string product = sysattr2string(usbdev, "manufacturer", "????") ;

        int vendorID = sysattr2int(usbdev, "idVendor") ;
        int productID = sysattr2int(usbdev, "idProduct") ;

        //string serial = sysattr2string(hiddev, "serial", "????") ;
        //cout << serial << endl;
        desc.name = vendor + " " + product;
        desc.vendorID = vendorID;
        desc.productID = productID;
    }

    void linuxPointingDeviceManager::checkFoundDevice(struct udev_device *hiddev)
    {
        struct udev_device *usbdev = udev_device_get_parent_with_subsystem_devtype(hiddev, "usb", "usb_device") ;
        if (!usbdev) return ;

        // std::cout << "Device added" << std::endl;
        PointingDeviceDescriptor desc;
        ConvertDevice(hiddev, usbdev, desc);
        //std::cerr << "osxPointingDeviceManager::AddDevice: adding " << desc.deviceURI.asString() << std::endl ;

        addDevice(desc);
  }

    void linuxPointingDeviceManager::checkLostDevice(struct udev_device *dev)
    {
        PointingDeviceDescriptor desc(uriStringFromDevice(dev));
        removeDevice(desc);
    }

    linuxPointingDeviceManager::~linuxPointingDeviceManager()
    {
        if (pthread_cancel(thread) < 0)
            perror("linuxPointingDeviceManager::~linuxPointingDeviceManager");
        udev_monitor_unref(monitor);
        udev_unref(udev);
    }
}
