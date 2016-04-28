/* -*- mode: c++ -*-
 * 
 * pointing/ntransferfunction.h
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

#ifndef NTRANSFERFUNCTION_H
#define NTRANSFERFUNCTION_H

#include <nan.h>
#include <pointing/transferfunctions/SubPixelFunction.h>
#include "npointingdevice.h"
#include "ndisplaydevice.h"

class NTransferFunction : public Nan::ObjectWrap
{
  public:
    static NAN_MODULE_INIT(Init);

  private:
	  explicit NTransferFunction(std::string uri, NPointingDevice *ninput, NDisplayDevice *noutput);
  	~NTransferFunction();

  	static NAN_METHOD(New);
  	
  	static NAN_METHOD(applyi);
    static NAN_METHOD(applyd);
    static NAN_METHOD(clearState);
    static NAN_METHOD(setSubPixeling);
    static NAN_METHOD(setHumanResolution);
    static NAN_METHOD(setCardinalitySize);

    static NAN_GETTER(getURI);
    static NAN_GETTER(getSubPixeling);
    static NAN_GETTER(getHumanResolution);
    static NAN_GETTER(getCardinality);
    static NAN_GETTER(getWidgetSize);

  	static Nan::Persistent<v8::Function> constructor;

  	pointing::SubPixelFunction *func;
    Nan::Persistent<v8::Value> nInput;
    Nan::Persistent<v8::Value> nOutput;
};

#endif // NTRANSFERFUNCTION_H