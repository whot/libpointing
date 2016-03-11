/* -*- mode: c++ -*-
 * 
 * pointing/ndisplaydevice.h
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

#ifndef NDISPLAYDEVICE_H
#define NDISPLAYDEVICE_H

#include <nan.h>
#include <pointing/pointing.h>

class NDisplayDevice : public Nan::ObjectWrap
{
  public:
    static NAN_MODULE_INIT(Init);

  private:
	  explicit NDisplayDevice(std::string uri);
  	~NDisplayDevice();

  	static NAN_METHOD(New);
    static NAN_GETTER(getSize);
    static NAN_GETTER(getBounds);
    static NAN_GETTER(getResolution);
    static NAN_GETTER(getRefreshRate);
    static NAN_GETTER(getURI);

  	static Nan::Persistent<v8::Function> constructor;

  	pointing::DisplayDevice *output;

    friend class NTransferFunction;
};

#endif // NDISPLAYDEVICE_H