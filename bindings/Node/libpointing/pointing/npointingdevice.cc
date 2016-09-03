/* -*- mode: c++ -*-
 * 
 * pointing/npointingdevice.cc
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

#include "npointingdevice.h"
#include <uv.h>

using namespace v8;
using namespace pointing;

Nan::Persistent<Function> NPointingDevice::constructor;

void NPointingDevice::callCallback(TimeStamp::inttime timestamp, int dx, int dy, int buttons)
{
  Nan::HandleScope scope;

  Local<Value> argv[] = {
          Nan::New<Number>(timestamp),
          Nan::New<Number>(dx),
          Nan::New<Number>(dy),
          Nan::New<Number>(buttons)
  };
  
  if (!callback.IsEmpty())
  {
    callback.Call(4, argv);
  }
}

void NPointingDevice::pointingCallback(void *context, TimeStamp::inttime timestamp, int dx, int dy, int buttons)
{
#ifdef __APPLE__
  NPointingDevice *self = (NPointingDevice *)context;
  self->callCallback(timestamp, dx, dy, buttons);
#else
  NPointingDevice *self = (NPointingDevice *)context;
  PointingReport *pr = new PointingReport();
  pr->timestamp = timestamp;
  pr->dx = dx;
  pr->dy = dy;
  pr->buttons = buttons;
  
  uv_mutex_lock(&self->pqueue_mutex);
  self->pqueue_.push(pr);
  uv_mutex_unlock(&self->pqueue_mutex);
  uv_async_send(&self->async_);
#endif
}

#ifndef __APPLE__

void NPointingDevice::asyncHandler(uv_async_t *handle)
{
  NPointingDevice *self = (NPointingDevice *)(handle->data);

  uv_mutex_lock(&self->pqueue_mutex);
  
  while (!self->pqueue_.empty())
  {
    PointingReport *pr = self->pqueue_.front();
    self->pqueue_.pop();
    self->callCallback(pr->timestamp, pr->dx, pr->dy, pr->buttons);
    delete pr;
  }

  uv_mutex_unlock(&self->pqueue_mutex);
}

#endif

NPointingDevice::NPointingDevice(std::string uri)
  :input(0)
{
  input = PointingDevice::create(uri);
#ifndef __APPLE__
  uv_mutex_init(&pqueue_mutex);
  uv_loop_t *loop = uv_default_loop();
  async_.data = this;
  
  uv_async_init(loop, &async_, (uv_async_cb)asyncHandler);
#endif
}

NPointingDevice::~NPointingDevice() {
  //std::cerr << "~NPointingDevice" << std::endl;
  delete input;
}

NAN_MODULE_INIT(NPointingDevice::Init)
{
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("PointingDevice").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(tpl, "setPointingCallback", setPointingCallback);

  Local<ObjectTemplate> itpl = tpl->InstanceTemplate();
  Nan::SetAccessor(itpl, Nan::New("vendorID").ToLocalChecked(), getVendorID);
  Nan::SetAccessor(itpl, Nan::New("productID").ToLocalChecked(), getProductID);
  Nan::SetAccessor(itpl, Nan::New("vendor").ToLocalChecked(), getVendor);
  Nan::SetAccessor(itpl, Nan::New("product").ToLocalChecked(), getProduct);
  Nan::SetAccessor(itpl, Nan::New("updateFrequency").ToLocalChecked(), getUpdateFrequency);
  Nan::SetAccessor(itpl, Nan::New("resolution").ToLocalChecked(), getResolution);
  Nan::SetAccessor(itpl, Nan::New("uri").ToLocalChecked(), getURI);
  Nan::SetAccessor(itpl, Nan::New("expandedUri").ToLocalChecked(), getExpandedURI);
  Nan::SetAccessor(itpl, Nan::New("active").ToLocalChecked(), isActive);

  constructor.Reset(Nan::GetFunction(tpl).ToLocalChecked());
  Nan::Set(target, Nan::New("PointingDevice").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());
}

NAN_METHOD(NPointingDevice::New)
{
  if (info.IsConstructCall()) {
    String::Utf8Value str(info[0]->ToString());
    std::string uri(*str);
    NPointingDevice* obj = new NPointingDevice(uri);
    obj->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  } 
  else 
  {
    Local<Value> argv[1] = { info[0] };
    Local<v8::Function> cons = Nan::New(constructor);
    info.GetReturnValue().Set(cons->NewInstance(1, argv));
  }
}

NAN_METHOD(NPointingDevice::setPointingCallback)
{
  NPointingDevice* self = ObjectWrap::Unwrap<NPointingDevice>(info.Holder());

  self->callback.SetFunction(info[0].As<Function>());
  if (self->callback.IsEmpty())
  {
    self->input->setPointingCallback(NULL, NULL);
    if (self->refs_) // Reference count with self->Ref()
      self->Unref();
  }
  else
  {
    if (!self->refs_)
      self->Ref();
    self->input->setPointingCallback(pointingCallback, self);
  }
  info.GetReturnValue().Set(info.Holder());
}

NAN_GETTER(NPointingDevice::getVendorID)
{
  NPointingDevice* self = ObjectWrap::Unwrap<NPointingDevice>(info.Holder());

  info.GetReturnValue().Set(Nan::New(self->input->getVendorID()));
}

NAN_GETTER(NPointingDevice::getProductID)
{
  NPointingDevice* self = ObjectWrap::Unwrap<NPointingDevice>(info.Holder());

  info.GetReturnValue().Set(Nan::New(self->input->getProductID()));
}

NAN_GETTER(NPointingDevice::getVendor)
{
  NPointingDevice* self = ObjectWrap::Unwrap<NPointingDevice>(info.Holder());

  info.GetReturnValue().Set(Nan::New(self->input->getVendor()).ToLocalChecked());
}

NAN_GETTER(NPointingDevice::getProduct)
{
  NPointingDevice* self = ObjectWrap::Unwrap<NPointingDevice>(info.Holder());

  info.GetReturnValue().Set(Nan::New(self->input->getProduct()).ToLocalChecked());
}

NAN_GETTER(NPointingDevice::getUpdateFrequency)
{
  NPointingDevice* self = ObjectWrap::Unwrap<NPointingDevice>(info.Holder());

  info.GetReturnValue().Set(Nan::New(self->input->getUpdateFrequency()));
}

NAN_GETTER(NPointingDevice::getResolution)
{
  NPointingDevice* self = ObjectWrap::Unwrap<NPointingDevice>(info.Holder());

  info.GetReturnValue().Set(Nan::New(self->input->getResolution()));
}

NAN_GETTER(NPointingDevice::getURI)
{
  NPointingDevice* self = ObjectWrap::Unwrap<NPointingDevice>(info.Holder());

  URI result = self->input->getURI();

  info.GetReturnValue().Set(Nan::New(result.asString()).ToLocalChecked());
}

NAN_GETTER(NPointingDevice::getExpandedURI)
{
  NPointingDevice* self = ObjectWrap::Unwrap<NPointingDevice>(info.Holder());

  URI result = self->input->getURI(true);

  info.GetReturnValue().Set(Nan::New(result.asString()).ToLocalChecked());
}

NAN_GETTER(NPointingDevice::isActive)
{
  NPointingDevice* self = ObjectWrap::Unwrap<NPointingDevice>(info.Holder());

  info.GetReturnValue().Set(Nan::New(self->input->isActive()));
}
