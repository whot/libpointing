/* -*- mode: c++ -*-
 * 
 * pointing/nsystempointeracceleration.cc
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
 
#include "nsystempointeracceleration.h"
#include <iostream>
#include <pointing/pointing.h>

#ifdef __APPLE__
#include <pointing/transferfunctions/osx/osxSystemPointerAcceleration.h>
#elif defined(__linux__)
#include <pointing/transferfunctions/linux/xorgSystemPointerAcceleration.h>
#elif defined(_WIN32)
#include <pointing/transferfunctions/windows/winSystemPointerAcceleration.h>
#endif

using namespace v8;
using namespace pointing;

Nan::Persistent<Function> NSystemPointerAcceleration::constructor;

NAN_METHOD(NSystemPointerAcceleration::get)
{
  Local<Object> result = Nan::New<Object>();
#ifdef __APPLE__
  double res = 0;
  osxSystemPointerAcceleration acc;
  Local<Value> tarStr = Nan::New("mouse").ToLocalChecked();
  if (info.Length())
  {
    Local<Object> argsObj = info[0]->ToObject();
    if (Nan::Has(argsObj, Nan::New("target").ToLocalChecked()).FromMaybe(false))
      tarStr = Nan::Get(argsObj, Nan::New("target").ToLocalChecked()).ToLocalChecked();
    String::Utf8Value target(tarStr->ToString());
    res = acc.get(*target);
  }
  else
    res = acc.get();

  result->Set(Nan::New("value").ToLocalChecked(), Nan::New(res));
  result->Set(Nan::New("target").ToLocalChecked(), tarStr);
  
#elif defined __linux__
	xorgSystemPointerAcceleration acc;
	int num = 2, den = 1, thr = 4;
	acc.get(&num, &den, &thr);
  result->Set(Nan::New("numerator").ToLocalChecked(), Nan::New(num));
  result->Set(Nan::New("denominator").ToLocalChecked(), Nan::New(den));
  result->Set(Nan::New("threshold").ToLocalChecked(), Nan::New(thr));
#elif defined(_WIN32)
  winSystemPointerAcceleration acc;
  std::string version;
  int sliderPosition;
  bool epp;
  acc.get(&version, &sliderPosition, &epp);
  result->Set(Nan::New("version").ToLocalChecked(), Nan::New(version).ToLocalChecked());
  result->Set(Nan::New("sliderPosition").ToLocalChecked(), Nan::New(sliderPosition));
  result->Set(Nan::New("enhancePointerPrecision").ToLocalChecked(), Nan::New(epp));
#endif
  info.GetReturnValue().Set(result);
}

NAN_METHOD(NSystemPointerAcceleration::set)
{
  Local<Object> argsObj = info[0]->ToObject();
#ifdef __APPLE__
  if (Nan::Has(argsObj, Nan::New("value").ToLocalChecked()).FromMaybe(false))
  {
    double value = Nan::Get(argsObj, Nan::New("value").ToLocalChecked()).ToLocalChecked()->NumberValue();
    Local<Value> tarStr = Nan::New("mouse").ToLocalChecked();
    if (Nan::Has(argsObj, Nan::New("target").ToLocalChecked()).FromMaybe(false))
      tarStr = Nan::Get(argsObj, Nan::New("target").ToLocalChecked()).ToLocalChecked();
    String::Utf8Value target(tarStr->ToString());
    osxSystemPointerAcceleration acc;
    acc.set(value, *target);
  }
#elif defined(__linux__)
  int num = Nan::Get(argsObj, Nan::New("numerator").ToLocalChecked()).ToLocalChecked()->IntegerValue();
  int den = Nan::Get(argsObj, Nan::New("denominator").ToLocalChecked()).ToLocalChecked()->IntegerValue();
  int thr = Nan::Get(argsObj, Nan::New("threshold").ToLocalChecked()).ToLocalChecked()->IntegerValue();
  
  num = num ? num : 2;
  den = den ? den : 1;
  thr = thr ? thr : 4;
  
	xorgSystemPointerAcceleration acc;
	acc.set(num, den, thr);
#elif defined(_WIN32)
  int sliderPosition = Nan::Get(argsObj, Nan::New("sliderPosition").ToLocalChecked()).ToLocalChecked()->IntegerValue();
  bool epp = Nan::Get(argsObj, Nan::New("enhancePointerPrecision").ToLocalChecked()).ToLocalChecked()->BooleanValue();
  winSystemPointerAcceleration acc;
  acc.set(sliderPosition, epp);
#endif
  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(NSystemPointerAcceleration::New)
{
  if (info.IsConstructCall()) {
    info.GetReturnValue().Set(info.This());
  } 
  else {
    Local<Function> cons = Nan::New<Function>(constructor);
    info.GetReturnValue().Set(cons->NewInstance(0, NULL));
  }
}

NAN_MODULE_INIT(NSystemPointerAcceleration::Init)
{
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("SystemPointerAcceleration").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(tpl, "get", get);
  Nan::SetPrototypeMethod(tpl, "set", set);

  constructor.Reset(Nan::GetFunction(tpl).ToLocalChecked());
  target->Set(Nan::New("SystemPointerAcceleration").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());
}
