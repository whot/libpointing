/* -*- mode: c++ -*-
 * 
 * pointing/npointingdevice.h
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

#ifndef NPOINTINGDEVICE_H
#define NPOINTINGDEVICE_H

#include <nan.h>
#include <pointing/pointing.h>
#ifndef __APPLE__
#include <queue>
#include <uv.h>
#endif

class NPointingDevice : public Nan::ObjectWrap
{
  public:
    static NAN_MODULE_INIT(Init);

  private:

    explicit NPointingDevice(std::string uri);
    ~NPointingDevice();

    static NAN_METHOD(New);

    static NAN_METHOD(setPointingCallback);

    static NAN_GETTER(getVendorID);
    static NAN_GETTER(getProductID);
    static NAN_GETTER(getVendor);
    static NAN_GETTER(getProduct);
    static NAN_GETTER(getUpdateFrequency);
    static NAN_GETTER(getResolution);
    static NAN_GETTER(getURI);
    static NAN_GETTER(isActive);

    static Nan::Persistent<v8::Function> constructor;
    Nan::Callback callback;
    
    pointing::PointingDevice *input;
    static void pointingCallback(void *context, pointing::TimeStamp::inttime timestamp, int dx, int dy, int buttons);

    friend class NTransferFunction;
    void callCallback(pointing::TimeStamp::inttime timestamp, int dx, int dy, int buttons);

#ifndef __APPLE__
  
    struct PointingReport
    {
      pointing::TimeStamp::inttime timestamp;
      int dx;
      int dy;
      int buttons;
    };

    static void asyncHandler(uv_async_t *handle);
    uv_async_t async_;
    
    uv_mutex_t pqueue_mutex;
    std::queue<PointingReport *> pqueue_;

#endif

};

#endif // NPOINTINGDEVICE_H