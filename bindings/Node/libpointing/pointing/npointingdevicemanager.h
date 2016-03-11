/* -*- mode: c++ -*-
 * 
 * pointing/npointingdevicemanager.h
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

#ifndef NPOINTINGDEVICEMANAGER_H
#define NPOINTINGDEVICEMANAGER_H

#include <nan.h>
#include <map>
#include <pointing/input/PointingDeviceManager.h>

class NPointingDeviceManager : public Nan::ObjectWrap
{
  public:
    static NAN_MODULE_INIT(Init);

  private:
  	static NAN_METHOD(New);

  	static NAN_METHOD(addDeviceUpdateCallback);
    static NAN_METHOD(removeDeviceUpdateCallback);

    static NAN_GETTER(getDeviceList);

  	static Nan::Persistent<v8::Function> constructor;

  	static void deviceUpdateCallback(void *context, const pointing::PointingDeviceDescriptor &descriptor, bool wasAdded);
};

#endif // NPOINTINGDEVICEMANAGER_H