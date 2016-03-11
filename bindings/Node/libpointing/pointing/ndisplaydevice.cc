/* -*- mode: c++ -*-
 * 
 * pointing/ndisplaydevice.cc
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

#include "ndisplaydevice.h"

bool custom_isnan(double var)
{
    volatile double d = var;
    return d != d;
}

using namespace v8;
using namespace pointing;

Nan::Persistent<Function> NDisplayDevice::constructor;

NDisplayDevice::NDisplayDevice(std::string uri) : output(0)
{
  output = DisplayDevice::create(uri);
}

NDisplayDevice::~NDisplayDevice()
{
  delete output;
}

NAN_MODULE_INIT(NDisplayDevice::Init)
{
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("DisplayDevice").ToLocalChecked());

  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Local<ObjectTemplate> itpl = tpl->InstanceTemplate();
  Nan::SetAccessor(itpl, Nan::New("size").ToLocalChecked(), getSize);
  Nan::SetAccessor(itpl, Nan::New("bounds").ToLocalChecked(), getBounds);
  Nan::SetAccessor(itpl, Nan::New("resolution").ToLocalChecked(), getResolution);
  Nan::SetAccessor(itpl, Nan::New("refreshRate").ToLocalChecked(), getRefreshRate);
  Nan::SetAccessor(itpl, Nan::New("uri").ToLocalChecked(), getURI);

  constructor.Reset(Nan::GetFunction(tpl).ToLocalChecked());
  Nan::Set(target, Nan::New("DisplayDevice").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());
}

NAN_METHOD(NDisplayDevice::New)
{
  if (info.IsConstructCall())
  {
    String::Utf8Value str(info[0]->ToString());
    std::string uri(*str);
    NDisplayDevice* obj = new NDisplayDevice(uri);
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

NAN_GETTER(NDisplayDevice::getSize)
{
  NDisplayDevice* self = ObjectWrap::Unwrap<NDisplayDevice>(info.Holder());

  DisplayDevice::Size size = self->output->getSize();
  Local<Object> result = Nan::New<Object>();
  Nan::Set(result, Nan::New("width").ToLocalChecked(), Nan::New(size.width));
  Nan::Set(result, Nan::New("height").ToLocalChecked(), Nan::New(size.height));

  info.GetReturnValue().Set(result);
}

NAN_GETTER(NDisplayDevice::getBounds)
{
  NDisplayDevice* self = ObjectWrap::Unwrap<NDisplayDevice>(info.Holder());

  DisplayDevice::Bounds bounds = self->output->getBounds();
  Local<Object> result = Nan::New<Object>();

  Local<Object> size = Nan::New<Object>();
  Nan::Set(size, Nan::New("width").ToLocalChecked(), Nan::New(bounds.size.width));
  Nan::Set(size, Nan::New("height").ToLocalChecked(), Nan::New(bounds.size.height));
  Nan::Set(result, Nan::New("size").ToLocalChecked(), size);

  Local<Object> origin = Nan::New<Object>();
  Nan::Set(origin, Nan::New("x").ToLocalChecked(), Nan::New(bounds.origin.x));
  Nan::Set(origin, Nan::New("y").ToLocalChecked(), Nan::New(bounds.origin.y));
  Nan::Set(result, Nan::New("origin").ToLocalChecked(), origin);

  info.GetReturnValue().Set(result);
}

NAN_GETTER(NDisplayDevice::getResolution)
{
  NDisplayDevice* self = ObjectWrap::Unwrap<NDisplayDevice>(info.Holder());

  double hppi = 0, vppi = 0;
  self->output->getResolution(&hppi, &vppi);
  if (custom_isnan(hppi) || custom_isnan(vppi))
  {
	  hppi = 0;
	  vppi = 0;
  }
  Local<Object> result = Nan::New<Object>();
  Nan::Set(result, Nan::New("hppi").ToLocalChecked(), Nan::New(hppi));
  Nan::Set(result, Nan::New("vppi").ToLocalChecked(), Nan::New(vppi));

  info.GetReturnValue().Set(result);
}

NAN_GETTER(NDisplayDevice::getRefreshRate)
{
  NDisplayDevice* self = ObjectWrap::Unwrap<NDisplayDevice>(info.Holder());

  double rr = self->output->getRefreshRate();

  info.GetReturnValue().Set(Nan::New(rr));
}

NAN_GETTER(NDisplayDevice::getURI)
{
  NDisplayDevice* self = ObjectWrap::Unwrap<NDisplayDevice>(info.Holder());

  URI result = self->output->getURI();

  info.GetReturnValue().Set(Nan::New(result.asString()).ToLocalChecked());
}