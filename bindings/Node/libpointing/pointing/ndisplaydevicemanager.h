/* -*- mode: c++ -*-
 * 
 * pointing/ndisplaydevicemanager.h
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
 
#ifndef NDISPLAYDEVICEMANAGER_H
#define NDISPLAYDEVICEMANAGER_H

#include <nan.h>
#include <map>
#include <pointing/output/DisplayDeviceManager.h>

class NDisplayDeviceManager : public Nan::ObjectWrap
{
  public:
    static NAN_MODULE_INIT(Init);

  private:
  	static NAN_METHOD(New);

  	static NAN_METHOD(addDeviceUpdateCallback);
    static NAN_METHOD(removeDeviceUpdateCallback);

    static NAN_GETTER(getDeviceList);

  	static Nan::Persistent<v8::Function> constructor;

  	static void deviceUpdateCallback(void *context, const pointing::DisplayDeviceDescriptor &descriptor, bool wasAdded);
};

#endif // NDISPLAYDEVICEMANAGER_H