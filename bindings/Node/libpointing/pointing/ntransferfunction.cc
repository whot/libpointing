/* -*- mode: c++ -*-
 * 
 * pointing/ntransferfunction.cc
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

#include "ntransferfunction.h"

using namespace v8;
using namespace pointing;

Nan::Persistent<Function> NTransferFunction::constructor;

NTransferFunction::NTransferFunction(std::string uri,
  NPointingDevice *ninput, NDisplayDevice *noutput) : func(0)
{
  func = new SubPixelFunction("subpixel:?isOn=false", uri, ninput->input, noutput->output);
}

NTransferFunction::~NTransferFunction()
{
  delete func;
}

NAN_MODULE_INIT(NTransferFunction::Init)
{
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("TransferFunction").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Local<ObjectTemplate> itpl = tpl->InstanceTemplate();
  Nan::SetAccessor(itpl, Nan::New("uri").ToLocalChecked(), getURI);
  Nan::SetAccessor(itpl, Nan::New("subPixeling").ToLocalChecked(), getSubPixeling);
  Nan::SetAccessor(itpl, Nan::New("humanResolution").ToLocalChecked(), getHumanResolution);
  Nan::SetAccessor(itpl, Nan::New("cardinality").ToLocalChecked(), getCardinality);
  Nan::SetAccessor(itpl, Nan::New("widgetSize").ToLocalChecked(), getWidgetSize);
  
  Nan::SetPrototypeMethod(tpl, "applyi", applyi);
  Nan::SetPrototypeMethod(tpl, "applyd", applyd);
  Nan::SetPrototypeMethod(tpl, "clearState", clearState);
  Nan::SetPrototypeMethod(tpl, "setSubPixeling", setSubPixeling);
  Nan::SetPrototypeMethod(tpl, "setHumanResolution", setHumanResolution);
  Nan::SetPrototypeMethod(tpl, "setCardinalitySize", setCardinalitySize);

  constructor.Reset(Nan::GetFunction(tpl).ToLocalChecked());
  Nan::Set(target, Nan::New("TransferFunction").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());
}

NAN_METHOD(NTransferFunction::New)
{
  if (info.IsConstructCall()) {
    // Invoked as constructor: `new MyObject(...)
    String::Utf8Value str(info[0]->ToString());
    std::string uri(*str);
    NPointingDevice *ninput = ObjectWrap::Unwrap<NPointingDevice>(info[1]->ToObject());
    NDisplayDevice *noutput = ObjectWrap::Unwrap<NDisplayDevice>(info[2]->ToObject());
    NTransferFunction* obj = new NTransferFunction(uri, ninput, noutput);
    obj->Wrap(info.This());

    info.GetReturnValue().Set(info.This());
  } else {
    // Invoked as plain function `MyObject(...)`, turn into construct call.
    const int argc = 3;
    Local<Value> argv[argc] = { info[0], info[1], info[2] };
    Local<Function> cons = Nan::New<Function>(constructor);
    info.GetReturnValue().Set(cons->NewInstance(argc, argv));
  }
}

NAN_METHOD(NTransferFunction::applyi)
{
  NTransferFunction* obj = ObjectWrap::Unwrap<NTransferFunction>(info.Holder());

  int dxPixel = 0, dyPixel = 0;
  int dx = info[0].As<Number>()->IntegerValue();
  int dy = info[1].As<Number>()->IntegerValue();
  TimeStamp::inttime timestamp = info[2].As<Number>()->IntegerValue();
  obj->func->applyi(dx, dy, &dxPixel, &dyPixel, timestamp);

  Local<Object> result = Nan::New<Object>();
  
  result->Set(Nan::New("dx").ToLocalChecked(), Nan::New<Number>(dxPixel));
  result->Set(Nan::New("dy").ToLocalChecked(), Nan::New<Number>(dyPixel));

  info.GetReturnValue().Set(result);
}

NAN_METHOD(NTransferFunction::applyd)
{
  NTransferFunction* obj = ObjectWrap::Unwrap<NTransferFunction>(info.Holder());

  double dxPixel = 0, dyPixel = 0;
  int dx = info[0].As<Number>()->IntegerValue();
  int dy = info[1].As<Number>()->IntegerValue();
  TimeStamp::inttime timestamp = info[2].As<Number>()->IntegerValue();
  obj->func->applyd(dx, dy, &dxPixel, &dyPixel, timestamp);

  Local<Object> result = Nan::New<Object>();
  
  result->Set(Nan::New("dx").ToLocalChecked(), Nan::New<Number>(dxPixel));
  result->Set(Nan::New("dy").ToLocalChecked(), Nan::New<Number>(dyPixel));

  info.GetReturnValue().Set(result);
}

NAN_METHOD(NTransferFunction::clearState)
{
  NTransferFunction* obj = ObjectWrap::Unwrap<NTransferFunction>(info.Holder());

  obj->func->clearState();

  info.GetReturnValue().Set(info.Holder());
}

NAN_METHOD(NTransferFunction::setSubPixeling)
{
  NTransferFunction* obj = ObjectWrap::Unwrap<NTransferFunction>(info.Holder());

  bool subpixeling = info[0].As<Number>()->BooleanValue();
  obj->func->setSubPixeling(subpixeling);

  info.GetReturnValue().Set(info.Holder());
}

NAN_METHOD(NTransferFunction::setHumanResolution)
{
  NTransferFunction* obj = ObjectWrap::Unwrap<NTransferFunction>(info.Holder());

  int humanResolution = info[0].As<Number>()->IntegerValue();
  obj->func->setHumanResolution(humanResolution);
  
  info.GetReturnValue().Set(info.Holder());
}

NAN_METHOD(NTransferFunction::setCardinalitySize)
{
  NTransferFunction* obj = ObjectWrap::Unwrap<NTransferFunction>(info.Holder());

  int cardinality = info[0].As<Number>()->IntegerValue();
  int size = info[1].As<Number>()->IntegerValue();
  obj->func->setCardinalitySize(cardinality, size);
  
  info.GetReturnValue().Set(info.Holder());
}

NAN_GETTER(NTransferFunction::getURI)
{
  NTransferFunction* self = ObjectWrap::Unwrap<NTransferFunction>(info.Holder());

  URI result = self->func->getInnerURI();

  info.GetReturnValue().Set(Nan::New(result.asString()).ToLocalChecked());
}

NAN_GETTER(NTransferFunction::getSubPixeling)
{
  NTransferFunction* obj = ObjectWrap::Unwrap<NTransferFunction>(info.Holder());

  bool subpixeling = obj->func->getSubPixeling();

  info.GetReturnValue().Set(Nan::New<Boolean>(subpixeling));
}

NAN_GETTER(NTransferFunction::getHumanResolution)
{
  NTransferFunction* obj = ObjectWrap::Unwrap<NTransferFunction>(info.Holder());

  int subpixeling = obj->func->getHumanResolution();

  info.GetReturnValue().Set(Nan::New(subpixeling));
}

NAN_GETTER(NTransferFunction::getCardinality)
{
  NTransferFunction* obj = ObjectWrap::Unwrap<NTransferFunction>(info.Holder());

  int cardinality = 0, size = 0;
  obj->func->getCardinalitySize(&cardinality, &size);

  info.GetReturnValue().Set(Nan::New(cardinality));
}

NAN_GETTER(NTransferFunction::getWidgetSize)
{
  NTransferFunction* obj = ObjectWrap::Unwrap<NTransferFunction>(info.Holder());

  int cardinality = 0, size = 0;
  obj->func->getCardinalitySize(&cardinality, &size);

  info.GetReturnValue().Set(Nan::New(size));
}