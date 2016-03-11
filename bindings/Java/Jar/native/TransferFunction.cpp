/* -*- mode: c++ -*-
 *
 * Bindings/Java/Jar/native/TransferFunction.cpp --
 *
 * Initial software
 * Authors: Géry Casiez, Izzat Mukhanov
 * Copyright © Inria
 *
 * http://libpointing.org/
 *
 * This software may be used and distributed according to the terms of
 * the GNU General Public License version 2 or any later version.
 *
 */

#include <jni.h>
#include <iostream>
#include "org_libpointing_TransferFunction.h"
#include "handle.h"

#include <pointing/pointing.h>

using namespace pointing;

JNIEXPORT jlong JNICALL Java_org_libpointing_TransferFunction_initTransferFunction
  (JNIEnv *env, jobject, jstring uriStr, jobject jinput, jobject joutput)
{
  PointingDevice *input = getHandle<PointingDevice>(env, jinput);
  DisplayDevice *output = getHandle<DisplayDevice>(env, joutput);
  const char *uri = env->GetStringUTFChars(uriStr, 0);
  TransferFunction *func = TransferFunction::create(uri, input, output);
  env->ReleaseStringUTFChars(uriStr, uri);
  return (jlong)func;
}

JNIEXPORT void JNICALL Java_org_libpointing_TransferFunction_releaseTransferFunction
  (JNIEnv *env, jobject obj)
{
  delete getHandle<TransferFunction>(env, obj);
}

JNIEXPORT jstring JNICALL Java_org_libpointing_TransferFunction_getURI
  (JNIEnv *env, jobject obj)
{
  TransferFunction *p = getHandle<TransferFunction>(env, obj);
  return env->NewStringUTF(p->getURI().asString().c_str());
}

JNIEXPORT jobject JNICALL Java_org_libpointing_TransferFunction_applyi
  (JNIEnv *env, jobject obj, jint inputDx, jint inputDy, jlong timestamp)
{
  jclass handlerClass = env->FindClass("java/awt/Point");
  jmethodID mid = env->GetMethodID(handlerClass, "setLocation", "(II)V");
  if (mid == NULL) {
    std::cerr << "Error : cannot find method set" << std::endl;
    return NULL;
  } else {
    int output_dx = 0, output_dy = 0;
    TransferFunction *func = getHandle<TransferFunction>(env, obj);
    func->applyi(inputDx, inputDy, &output_dx, &output_dy, timestamp);
    jint joutput_dx = output_dx;
    jint joutput_dy = output_dy;
    return env->NewObject(handlerClass, mid, joutput_dx, joutput_dy);
  }
}

JNIEXPORT jobject JNICALL Java_org_libpointing_TransferFunction_applyd
  (JNIEnv *env, jobject obj, jint inputDx, jint inputDy, jlong timestamp)
{
  jclass handlerClass = env->FindClass("java/awt/geom/Point2D$Double");
  jmethodID mid = env->GetMethodID(handlerClass, "setLocation", "(DD)V");
  if (mid == NULL) {
    std::cerr << "Error : cannot find method set" << std::endl;
    return NULL;
  } else {
    double output_dx = 0.0, output_dy = 0.0;
    TransferFunction *func = getHandle<TransferFunction>(env, obj);
    func->applyd(inputDx, inputDy, &output_dx, &output_dy, timestamp);
    jdouble joutput_dx = output_dx;
    jdouble joutput_dy = output_dy;
    return env->NewObject(handlerClass, mid, joutput_dx, joutput_dy);
  }
}


JNIEXPORT void JNICALL Java_org_libpointing_TransferFunction_clearState
  (JNIEnv *env, jobject obj)
{
  getHandle<TransferFunction>(env, obj)->clearState();
}
