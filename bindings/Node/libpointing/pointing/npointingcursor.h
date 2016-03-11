/* -*- mode: c++ -*-
 * 
 * pointing/npointingcursor.h
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
 
#ifndef NPOINTINGCURSOR_H
#define NPOINTINGCURSOR_H

#include <nan.h>
#include <pointing/utils/PointingCursor.h>

namespace NPointingCursor
{
  NAN_METHOD(getPosition) {
    v8::Local<v8::Object> result = Nan::New<v8::Object>();
    double x, y;
    pointing::PointingCursor::getPosition(&x, &y);
    result->Set(Nan::New("x").ToLocalChecked(), Nan::New(x));
    result->Set(Nan::New("y").ToLocalChecked(), Nan::New(y));
    info.GetReturnValue().Set(result);
  }

  NAN_METHOD(setPosition) {
    double x = info[0]->NumberValue();
    double y = info[1]->NumberValue();
    pointing::PointingCursor::setPosition(x, y);
  }

  NAN_MODULE_INIT(Init)
  {
    v8::Local<v8::Object> result = Nan::New<v8::Object>();
    v8::Local<v8::FunctionTemplate> getP = Nan::New<v8::FunctionTemplate>(getPosition);
    v8::Local<v8::FunctionTemplate> setP = Nan::New<v8::FunctionTemplate>(setPosition);

    result->Set(Nan::New("getPosition").ToLocalChecked(), Nan::GetFunction(getP).ToLocalChecked());
    result->Set(Nan::New("setPosition").ToLocalChecked(), Nan::GetFunction(setP).ToLocalChecked());

    target->Set(Nan::New("pointingCursor").ToLocalChecked(), result);
  }
}

#endif // NPOINTINGCURSOR_H