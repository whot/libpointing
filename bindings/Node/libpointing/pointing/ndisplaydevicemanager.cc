/* -*- mode: c++ -*-
 * 
 * pointing/ndisplaydevicemanager.cc
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

#include "ndisplaydevicemanager.h"
#include <iostream>
#include <uv.h>
#ifndef __APPLE__
#include <queue>
#endif

using namespace v8;
using namespace pointing;

#ifndef __APPLE__

struct DisplayDescriptorCallback
{
  DisplayDeviceDescriptor desc;
  bool wasAdded;
};

static uv_async_t async_;
static uv_mutex_t pqueue_mutex;
static std::queue<DisplayDescriptorCallback *> pqueue_;

#endif

Nan::Persistent<Function> NDisplayDeviceManager::constructor;
typedef std::map<std::string, Nan::Callback *> callbackMap_t;
static callbackMap_t callbackMap = callbackMap_t();

void createDescObject(Local<Object> &descObj, const DisplayDeviceDescriptor &ddd)
{
  Nan::Set(descObj, Nan::New("devURI").ToLocalChecked(), Nan::New(ddd.devURI).ToLocalChecked());
  Nan::Set(descObj, Nan::New("name").ToLocalChecked(), Nan::New(ddd.name).ToLocalChecked());
}

void callAllCallbacks(const DisplayDeviceDescriptor &descriptor, bool wasAdded)
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

void NDisplayDeviceManager::deviceUpdateCallback(void *context, const DisplayDeviceDescriptor &descriptor, bool wasAdded)
{
#ifdef __APPLE__
  callAllCallbacks(descriptor, wasAdded);
#else
  DisplayDescriptorCallback *ddc = new DisplayDescriptorCallback;
  ddc->desc = descriptor;
  ddc->wasAdded = wasAdded;
  uv_mutex_lock(&pqueue_mutex);
  pqueue_.push(ddc);
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
    DisplayDescriptorCallback *ddc = pqueue_.front();
    pqueue_.pop();
    callAllCallbacks(ddc->desc, ddc->wasAdded);
    delete ddc;
  }

  uv_mutex_unlock(&pqueue_mutex);
}

#endif

NAN_MODULE_INIT(NDisplayDeviceManager::Init)
{
  DisplayDeviceManager::get()->addDeviceUpdateCallback(deviceUpdateCallback, NULL);

  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("DisplayDeviceManager").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(tpl, "addDeviceUpdateCallback", addDeviceUpdateCallback);
  Nan::SetPrototypeMethod(tpl, "removeDeviceUpdateCallback", removeDeviceUpdateCallback);

  Local<ObjectTemplate> itpl = tpl->InstanceTemplate();
  Nan::SetAccessor(itpl, Nan::New("deviceList").ToLocalChecked(), getDeviceList);

  constructor.Reset(Nan::GetFunction(tpl).ToLocalChecked());
  Nan::Set(target, Nan::New("DisplayDeviceManager").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());

#ifndef __APPLE__
  uv_mutex_init(&pqueue_mutex);
  uv_loop_t *loop = uv_default_loop();
  uv_async_init(loop, &async_, (uv_async_cb)asyncHandler);
#endif
}

NAN_METHOD(NDisplayDeviceManager::New)
{
  if (info.IsConstructCall()) {
    // Invoked as constructor: `new MyObject(...)`
    info.GetReturnValue().Set(info.This());
  } else {
    // Invoked as plain function `MyObject(...)`, turn into construct call.
    Local<Function> cons = Nan::New<Function>(constructor);
    info.GetReturnValue().Set(cons->NewInstance(0, NULL));
  }
}

NAN_METHOD(NDisplayDeviceManager::addDeviceUpdateCallback)
{
  callbackMap[*String::Utf8Value(info[0].As<String>())] = new Nan::Callback(info[0].As<Function>());

  info.GetReturnValue().Set(info.This());
}


NAN_METHOD(NDisplayDeviceManager::removeDeviceUpdateCallback)
{
  callbackMap.erase(*String::Utf8Value(info[0].As<String>()));
  
  info.GetReturnValue().Set(info.This());
}

NAN_GETTER(NDisplayDeviceManager::getDeviceList)
{
  DisplayDeviceManager *man = DisplayDeviceManager::get();
  // Creating javascript array
  Local<Array> result = Nan::New<Array>(man->size());

  int i = 0;
  for (DisplayDescriptorIterator it = man->begin(); it != man->end(); it++)
  {
    DisplayDeviceDescriptor pdd = *it;
    Local<Object> descObj = Nan::New<Object>();
    createDescObject(descObj, pdd);
    Nan::Set(result, i++, descObj);
  }
  info.GetReturnValue().Set(result);
}
