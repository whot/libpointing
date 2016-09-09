/* -*- mode: c++ -*-
 * 
 * pointing/npointingdevicemanager.cc
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
 
#include "npointingdevicemanager.h"
#include <iostream>
#include <uv.h>
#ifndef __APPLE__
#include <queue>
#endif

using namespace v8;
using namespace pointing;

#ifndef __APPLE__

struct PointingDescriptorCallback
{
  PointingDeviceDescriptor desc;
  bool wasAdded;
};

static uv_async_t async_;
static uv_mutex_t pqueue_mutex;
static std::queue<PointingDescriptorCallback *> pqueue_;

#endif

Nan::Persistent<Function> NPointingDeviceManager::constructor;
typedef std::map<std::string, Nan::Callback *> callbackMap_t;
static callbackMap_t callbackMap = callbackMap_t();

void createDescObject(Local<Object> &descObj, const PointingDeviceDescriptor &pdd)
{
  Nan::Set(descObj, Nan::New("devURI").ToLocalChecked(), Nan::New(pdd.devURI.asString()).ToLocalChecked());
  Nan::Set(descObj, Nan::New("vendor").ToLocalChecked(), Nan::New(pdd.vendor).ToLocalChecked());
  Nan::Set(descObj, Nan::New("product").ToLocalChecked(), Nan::New(pdd.product).ToLocalChecked());
  Nan::Set(descObj, Nan::New("vendorID").ToLocalChecked(), Nan::New(pdd.vendorID));
  Nan::Set(descObj, Nan::New("productID").ToLocalChecked(), Nan::New(pdd.productID));
}

void callAllCallbacks(const PointingDeviceDescriptor &descriptor, bool wasAdded)
{
  Nan::HandleScope scope;
  
  callbackMap_t::iterator it = callbackMap.begin();
  for(; it != callbackMap.end(); it++)
  {
    Local<Object> descObj = Nan::New<Object>();
    createDescObject(descObj, descriptor);
    Local<Value> argv[] = {
      descObj, Nan::New<Number>(wasAdded)
    };
    it->second->Call(2, argv);
  }
}

void NPointingDeviceManager::deviceUpdateCallback(void *context, const PointingDeviceDescriptor &descriptor, bool wasAdded)
{
#ifdef __APPLE__
  callAllCallbacks(descriptor, wasAdded);
#else
  PointingDescriptorCallback *pdc = new PointingDescriptorCallback;
  pdc->desc = descriptor;
  pdc->wasAdded = wasAdded;
  uv_mutex_lock(&pqueue_mutex);
  pqueue_.push(pdc);
  uv_mutex_unlock(&pqueue_mutex);
  uv_async_send(&async_);
#endif
}

#ifndef __APPLE__

static void asyncHandler(uv_async_t *handle)
{
  uv_mutex_lock(&pqueue_mutex);
  
  while (!pqueue_.empty())
  {
    PointingDescriptorCallback *pdc = pqueue_.front();
    pqueue_.pop();
    callAllCallbacks(pdc->desc, pdc->wasAdded);
    delete pdc;
  }

  uv_mutex_unlock(&pqueue_mutex);
}

#endif

NAN_MODULE_INIT(NPointingDeviceManager::Init)
{
  PointingDeviceManager::get()->addDeviceUpdateCallback(deviceUpdateCallback, NULL);

  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("PointingDeviceManager").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(tpl, "addDeviceUpdateCallback", addDeviceUpdateCallback);
  Nan::SetPrototypeMethod(tpl, "removeDeviceUpdateCallback", removeDeviceUpdateCallback);

  Local<ObjectTemplate> itpl = tpl->InstanceTemplate();
  Nan::SetAccessor(itpl, Nan::New("deviceList").ToLocalChecked(), getDeviceList);

  constructor.Reset(Nan::GetFunction(tpl).ToLocalChecked());
  Nan::Set(target, Nan::New("PointingDeviceManager").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());

#ifndef __APPLE__
  uv_mutex_init(&pqueue_mutex);
  uv_loop_t *loop = uv_default_loop();
  uv_async_init(loop, &async_, (uv_async_cb)asyncHandler);
#endif
}

NAN_METHOD(NPointingDeviceManager::New)
{
  if (info.IsConstructCall()) {
    // Invoked as constructor: `new MyObject(...)`
    info.GetReturnValue().Set(info.This());
  } else {
    // Invoked as plain function `MyObject(...)`, turn into construct call.
    Local<Function> cons = Nan::New<Function>(constructor);
    info.GetReturnValue().Set(Nan::NewInstance(cons).ToLocalChecked());
  }
}

NAN_METHOD(NPointingDeviceManager::addDeviceUpdateCallback)
{
  callbackMap[*String::Utf8Value(info[0].As<String>())] = new Nan::Callback(info[0].As<Function>());

  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(NPointingDeviceManager::removeDeviceUpdateCallback) {
 
  callbackMap.erase(*String::Utf8Value(info[0].As<String>()));
  
  info.GetReturnValue().Set(info.This());
}

NAN_GETTER(NPointingDeviceManager::getDeviceList)
{
  PointingDeviceManager *man = PointingDeviceManager::get();
  // Creating javascript array
  Local<Array> result = Nan::New<Array>(man->size());

  int i = 0;
  for (PointingDescriptorIterator it = man->begin(); it != man->end(); it++)
  {
    PointingDeviceDescriptor pdd = *it;
    Local<Object> descObj = Nan::New<Object>();
    createDescObject(descObj, pdd);
    Nan::Set(result, i++, descObj);
  }
  info.GetReturnValue().Set(result);
}
