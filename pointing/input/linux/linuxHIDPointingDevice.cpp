/* -*- mode: c++ -*-
 *
 * pointing/input/linux/linuxHIDPointingDevice.cpp --
 *
 * Initial software
 * Authors: Nicolas Roussel
 * Copyright © Inria
 *
 * http://libpointing.org/
 *
 * This software may be used and distributed according to the terms of
 * the GNU General Public License version 2 or any later version.
 *
 */

#include <pointing/utils/Base64.h>
#include <pointing/input/linux/linuxHIDPointingDevice.h>
#include <pointing/input/PointingDeviceManager.h>
#include <unistd.h>

#include <X11/Xatom.h>
#include <X11/extensions/XInput.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>

#include <linux/input.h>
#include <linux/hidraw.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include <iostream>
#include <stdexcept>

namespace pointing {

  #define XORG_DEFAULT_CPI       400.0
  #define XORG_DEFAULT_HZ        125.0

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


  // --------------------------------------------------------------------------

  static inline void
  udevDebugDevice(struct udev_device *dev, std::ostream& out) {
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

  static inline std::wstring
  sysattr2wstring(struct udev_device *dev, const char *key) {
    const char *value = udev_device_get_sysattr_value(dev, key) ;
    if (value) {
      size_t size = strlen(value) + 1 ;
      wchar_t wcstring[size] ;
      mbstowcs(wcstring, value, size) ;
      return wcstring ;
    }
    return std::wstring() ;
  }

  static inline std::string
  sysattr2string(struct udev_device *dev, const char *key, const char *defval=0) {
    const char *value = udev_device_get_sysattr_value(dev, key) ;
    return value ? value : (defval ? defval : std::string()) ;
  }

  static inline int str16ToInt(std::string value)
  {
      std::stringstream ss;
      ss << std::hex << value;
      int result;
      ss >> result;
      return result;
  }

  static inline std::string
  bustype2string(int bustype) {
    const char *names[] = {"???", "PCI", "ISAPNP", "USB", "HIL", "BLUETOOTH", "VIRTUAL"} ;
    if (bustype<0 || bustype>BUS_VIRTUAL) return names[0] ;
    return names[bustype] ;
  }

  // --------------------------------------------------------------------------

  linuxHIDPointingDevice::linuxHIDPointingDevice(URI uri):
    vendorID(0),productID(0),parser(0),reportLength(0),seizeDevice(0)
  {
    udev = udev_new() ;
    if (!udev)
      throw std::runtime_error("linuxHIDPointingDevice: udev_new failed") ;

    monitor = udev_monitor_new_from_netlink(udev, "udev") ;
    udev_monitor_filter_add_match_subsystem_devtype(monitor, "hidraw", NULL) ;

    debugLevel = 0 ;
    URI::getQueryArg(uri.query, "debugLevel", &debugLevel) ;
    URI::getQueryArg(uri.query, "seize", &seizeDevice) ;

    forced_cpi = forced_hz = -1.0 ;
    URI::getQueryArg(uri.query, "cpi", &forced_cpi) ;
    URI::getQueryArg(uri.query, "hz", &forced_hz) ;

    PointingDeviceManager *man = PointingDeviceManager::get();

    if (uri.scheme == "any")
    {
        URI::getQueryArg(uri.query, "vendor", &vendorID) ;
        URI::getQueryArg(uri.query, "product", &productID) ;
        uri = man->anyToSpecific(uri);
    }

    hid = -1 ;
    this->uri = PointingDeviceManager::generalizeAny(uri) ;

    parser = new HIDReportParser(NULL, 0, debugLevel);

    if (debugLevel)
    {
      for (PointingDescriptorIterator it = man->begin(); it != man->end(); it++)
      {
          PointingDeviceDescriptor desc = *it;
          URI descURI(desc.devURI);
          bool match = (descURI.path == this->uri.path);
          std::cout << (match ? "+ " : "  ") << desc.devURI
               << " [" << std::hex << "vend:0x" << desc.vendorID
               << ", prod:0x" << desc.productID
               << std::dec << " - " << desc.name << " ]" << std::endl;
      }
    }

    callback = 0 ;
    callback_context = 0 ;

    int ret = pthread_create(&thread, NULL, eventloop, (void*)this) ;
    if (ret<0) {
      perror("linuxHIDPointingDevice::linuxHIDPointingDevice") ;
      throw std::runtime_error("linuxHIDPointingDevice: pthread_create failed") ;
    }
  }

  void linuxHIDPointingDevice::enableDevice(bool value)
  {
    if (vendor.size() && product.size())
    {
      Display *dpy = XOpenDisplay(0);
      std::string fullName = vendor + " " + product;
      XDeviceInfo *info = find_device_info(dpy, (char *)fullName.c_str(), True);
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
  }

  bool
  linuxHIDPointingDevice::isActive(void) const {
    return hid!=-1 ;
  }

  int linuxHIDPointingDevice::getVendorID() const
  {
      return vendorID;
  }

  std::string linuxHIDPointingDevice::getVendor() const
  {
      return vendor;
  }

  int linuxHIDPointingDevice::getProductID() const
  {
      return productID;
  }

  std::string linuxHIDPointingDevice::getProduct() const
  {
      return product;
  }

  double
  linuxHIDPointingDevice::getResolution(double *defval) const {
    if (forced_cpi > 0) return forced_cpi;
    // TODO If possible find the correct value in the descriptor
    return defval ? *defval : XORG_DEFAULT_CPI;
  }

  double
  linuxHIDPointingDevice::getUpdateFrequency(double *defval) const {
    if (forced_hz > 0) return forced_hz;
    double estimated = estimatedUpdateFrequency();
    if (estimated > 0.)
      return estimated;
    return defval ? *defval : XORG_DEFAULT_HZ;
  }

  URI
  linuxHIDPointingDevice::getURI(bool expanded, bool crossplatform) const {
    URI result;

    if (crossplatform)
    {
        result.scheme = "any";
        if (vendorID)
            URI::addQueryArg(result.query, "vendor", vendorID) ;
        if (productID)
            URI::addQueryArg(result.query, "product", productID) ;
    }
    else
        result = uri;

    if (expanded || debugLevel)
        URI::addQueryArg(result.query, "debugLevel", debugLevel) ;
    if (expanded || seizeDevice)
        URI::addQueryArg(result.query, "seize", seizeDevice) ;

    if (expanded || forced_cpi > 0)
        URI::addQueryArg(result.query, "cpi", getResolution()) ;
    if (expanded || forced_hz > 0)
        URI::addQueryArg(result.query, "hz", getUpdateFrequency()) ;

    return result ;
  }

  void
  linuxHIDPointingDevice::setPointingCallback(PointingCallback cbck, void *ctx) {
    callback = cbck ;
    callback_context = ctx ;
  }

  void
  linuxHIDPointingDevice::setDebugLevel(int level) {
    debugLevel = level ;
  }

  linuxHIDPointingDevice::~linuxHIDPointingDevice() {
    if (seizeDevice)
      enableDevice(true);
    if (pthread_cancel(thread)<0)
      perror("linuxHIDPointingDevice::~linuxHIDPointingDevice") ;
    if (hid!=-1) close(hid) ;
    udev_monitor_unref(monitor) ;
    udev_unref(udev) ;
    delete parser;
  }

  // --------------------------------------------------------------------------

  /**
   * @brief It is called at the initialization
   * in an another thread. It checks the state of the Pointing Device.
   * @param context The pointer to the object itself
   * @return
   */
  void *
  linuxHIDPointingDevice::eventloop(void *context) {
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL) ;
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL) ;

    linuxHIDPointingDevice *self = (linuxHIDPointingDevice*)context ;

    // std::cerr << "linuxHIDPointingDevice::eventloop: enumerating devices" << std::endl ;
    struct udev_enumerate *enumerate = udev_enumerate_new(self->udev) ;
    udev_enumerate_add_match_subsystem(enumerate, "hidraw") ;
    udev_enumerate_scan_devices(enumerate) ;
    struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate) ;
    struct udev_list_entry *dev_list_entry ;
    udev_list_entry_foreach(dev_list_entry, devices) {
      const char *path = udev_list_entry_get_name(dev_list_entry) ;
      struct udev_device *dev = udev_device_new_from_syspath(self->udev, path) ;
      self->checkFoundDevice(dev) ;
      udev_device_unref(dev) ;
      if (self->hid!=-1) break ;
    }
    udev_enumerate_unref(enumerate) ;

    udev_monitor_enable_receiving(self->monitor) ;
    int monfd = udev_monitor_get_fd(self->monitor) ;
    while (true) {
      fd_set rfds, wfds, efds ;
      FD_ZERO(&rfds) ;
      FD_ZERO(&wfds) ;
      FD_ZERO(&efds) ;
      FD_SET(monfd, &rfds) ;
      if (self->hid!=-1) FD_SET(self->hid, &rfds) ;
      int nfds = (monfd>self->hid ? monfd : self->hid) + 1 ;
       std::cerr << "linuxHIDPointingDevice::eventloop: calling select" << std::endl ;
      int nbready = select(nfds,&rfds,&wfds,&efds,0) ;
       std::cerr << "linuxHIDPointingDevice::eventloop: pthread_cancel" << std::endl ;
      pthread_testcancel() ;
      if (nbready==-1)
    perror("linuxHIDPointingDevice::eventloop") ;
      else {
    if (FD_ISSET(monfd, &rfds)) self->monitor_readable() ;
    if (self->hid!=-1 && FD_ISSET(self->hid, &rfds)) self->hid_readable() ;
      }
    }

    return 0 ;
  }

  // --------------------------------------------------------------------------

  void
  linuxHIDPointingDevice::monitor_readable(void) {
    struct udev_device *dev = udev_monitor_receive_device(monitor) ;
    if (dev) {
      const char *action = udev_device_get_action(dev) ;
      if (!strcmp(action,"add")) {
    if (hid==-1) checkFoundDevice(dev) ;
      } else if (!strcmp(action,"remove")) {
    if (hid!=-1) checkLostDevice(dev) ;
      }
      udev_device_unref(dev) ;
    }
  }

  void
  linuxHIDPointingDevice::checkFoundDevice(struct udev_device *hiddev) {
    struct udev_device *usbdev = udev_device_get_parent_with_subsystem_devtype(hiddev, "usb", "usb_device") ;
    if (!usbdev) return ;

    std::string idVendor = sysattr2string(usbdev, "idVendor", "????") ;
    std::string idProduct = sysattr2string(usbdev, "idProduct", "????") ;
    const char *devnode = udev_device_get_devnode(hiddev) ;

    URI devUri ;
    devUri.scheme = "hidraw" ;
    devUri.path = devnode ;

    bool match = hid==-1 && (uri.isEmpty() || uri.resemble(devUri)) ;

    if (!match) return ;

    productID = str16ToInt(idProduct) ;
    vendorID = str16ToInt(idVendor) ;
    product = sysattr2string(usbdev, "product", "????") ;
    vendor = sysattr2string(usbdev, "manufacturer", "????") ;

    if (seizeDevice)
      enableDevice(false);

    hid = open(devnode, O_RDONLY) ;
    if (hid==-1) {
      perror("linuxHIDPointingDevice::checkFoundDevice") ;
      std::cerr << "linuxHIDPointingDevice::checkFoundDevice: unable to open HID device" << std::endl ;
      return ;
    }

#if 1
    // See http://kerneltrap.org/mailarchive/linux-kernel/2010/6/18/4584473

    int res ;
    char buffer[512] ;

    int descSize = 0 ;
    res = ioctl(hid, HIDIOCGRDESCSIZE, &descSize) ;
    if (res < 0) {
      perror("linuxHIDPointingDevice::checkFoundDevice") ;
      std::cerr << "linuxHIDPointingDevice::checkFoundDevice: unable to open HID device" << std::endl ;
      return ;
    }
    if (debugLevel>0)
      std::cerr << "  descriptor size: " << descSize << std::endl ;
    struct hidraw_report_descriptor descriptor ;
    descriptor.size = descSize ;
    res = ioctl(hid, HIDIOCGRDESC, &descriptor) ;
    if (!parser->setDescriptor(descriptor.value, descSize))
      std::cerr << "linuxHIDPointingDevice::checkFoundDevice: unable to parse the HID report descriptor" << std::endl;
    reportLength = parser->getReportLength();
    if (res < 0) {
      perror("linuxHIDPointingDevice::checkFoundDevice") ;
      return ;
    } else {
      if (debugLevel>1) {
    std::cerr << "  descriptor (" << descriptor.size << " bytes): " ;
    std::string reportstring ;
    reportstring.assign((const char *)descriptor.value, descriptor.size) ;
    std::cerr << Base64::encode(reportstring) << std::endl ;
#if 0
    std::cerr << std::hex ;
    for (unsigned i = 0; i < descriptor.size; ++i)
      std::cerr << " " << (int)descriptor.value[i] ;
    std::cerr << std::dec << std::endl ;
#endif
      }
    }

    // This returns something like "HID 04b3:3105". Not really useful...
    res = ioctl(hid, HIDIOCGRAWNAME(512), buffer) ;
    if (res < 0)
      perror("linuxHIDPointingDevice::checkFoundDevice") ;
    else if (debugLevel > 0)
      std::cerr << "  raw name: " << buffer << std::endl ;

    // This returns someting like "usb-0000:02:00.0-1/input0". Not really useful again...
    res = ioctl(hid, HIDIOCGRAWPHYS(512), buffer) ;
    if (res < 0)
      perror("linuxHIDPointingDevice::checkFoundDevice");
    else if (debugLevel > 0)
      std::cerr << "  physical address: " << buffer << std::endl ;

    struct hidraw_devinfo info ;
    // This returns something like "bustype: 3 (USB), vendor: 4b3, product: 3105". Not really useful, again...
    res = ioctl(hid, HIDIOCGRAWINFO, &info) ;
    if (res < 0)
      perror("linuxHIDPointingDevice::checkFoundDevice");
    else if (debugLevel > 0) {
      std::cerr << "  raw info:" << std::endl
        << "    bustype: " << info.bustype << " (" << bustype2string(info.bustype) << ")" << std::endl
        << std::hex
        << "    vendor: " << info.vendor << std::endl
        << "    product: " << info.product << std::endl
        << std::dec ;
    }
#endif

    uri = devUri ;
    if (debugLevel > 0)
      std::cerr << "linuxHIDPointingDevice::checkFoundDevice: found " << uri.path << std::endl ;
  }

  void
  linuxHIDPointingDevice::checkLostDevice(struct udev_device *dev) {
    if (hid==-1) return ;

    const char *devnode = udev_device_get_devnode(dev) ;
    if (uri.path==devnode) {
      if (debugLevel>0) std::cerr << "- " << devnode << std::endl ;
      close(hid) ;
      hid = -1 ;
    }
  }

  // --------------------------------------------------------------------------

  void
  linuxHIDPointingDevice::hid_readable(void) {
    if (hid==-1) return ;

    TimeStamp::inttime now = TimeStamp::createAsInt() ;
    unsigned char *report = new unsigned char[reportLength];
    int32_t length = read(hid, report, reportLength);
    parser->setReport(report);
    delete report;

    if (length > 0)
    {
      registerTimestamp(now);
      if (callback)
      {
        int dx=0, dy=0, buttons=0;
        parser->getReportData(&dx, &dy, &buttons);
        callback(callback_context, now, dx, dy, buttons) ;
      }
    }
  }

  // --------------------------------------------------------------------------

}
