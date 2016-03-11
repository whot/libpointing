/* -*- mode: c++ -*-
 * 
 * pointing/nsystempointeracceleration.h
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
 
#ifndef NSYSTEMPOINTERACCELERATION_H
#define NSYSTEMPOINTERACCELERATION_H

#include <nan.h>

class NSystemPointerAcceleration : public Nan::ObjectWrap
{
  public:
    static NAN_MODULE_INIT(Init);

  private:
  	static NAN_METHOD(New);

  	static NAN_METHOD(get);
  	static NAN_METHOD(set);

  	static Nan::Persistent<v8::Function> constructor;
};

#endif // NSYSTEMPOINTERACCELERATION_H