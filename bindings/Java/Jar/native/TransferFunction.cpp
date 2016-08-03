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

#include <pointing/transferfunctions/SubPixelFunction.h>

using namespace pointing;

JNIEXPORT jlong JNICALL Java_org_libpointing_TransferFunction_initTransferFunction
  (JNIEnv *env, jobject, jstring uriStr, jobject jinput, jobject joutput)
{
  PointingDevice *input = getHandle<PointingDevice>(env, jinput);
  DisplayDevice *output = getHandle<DisplayDevice>(env, joutput);
  const char *uri = env->GetStringUTFChars(uriStr, 0);

  SubPixelFunction *func = new SubPixelFunction("subpixel:?isOn=false", uri, input, output);
  env->ReleaseStringUTFChars(uriStr, uri);
  return (jlong)func;
}

JNIEXPORT void JNICALL Java_org_libpointing_TransferFunction_releaseTransferFunction
  (JNIEnv *env, jobject obj)
{
  delete getHandle<SubPixelFunction>(env, obj);
}

JNIEXPORT jstring JNICALL Java_org_libpointing_TransferFunction_getURI
  (JNIEnv *env, jobject obj)
{
  SubPixelFunction *p = getHandle<SubPixelFunction>(env, obj);
  return env->NewStringUTF(p->getInnerURI().asString().c_str());
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
    SubPixelFunction *func = getHandle<SubPixelFunction>(env, obj);
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
  getHandle<SubPixelFunction>(env, obj)->clearState();
}


JNIEXPORT void JNICALL Java_org_libpointing_TransferFunction_setSubPixeling
  (JNIEnv *env, jobject obj, jboolean subPixeling)
{
  getHandle<SubPixelFunction>(env, obj)->setSubPixeling(subPixeling);
}


JNIEXPORT void JNICALL Java_org_libpointing_TransferFunction_setHumanResolution
  (JNIEnv *env, jobject obj, jint humRes)
{
  getHandle<SubPixelFunction>(env, obj)->setHumanResolution(humRes);
}


JNIEXPORT void JNICALL Java_org_libpointing_TransferFunction_setCardinalitySize
  (JNIEnv *env, jobject obj, jint cardinality, jint widgetSize)
{
  getHandle<SubPixelFunction>(env, obj)->setCardinalitySize(cardinality, widgetSize);
}


JNIEXPORT jboolean JNICALL Java_org_libpointing_TransferFunction_getSubPixeling
  (JNIEnv *env, jobject obj)
{
  return getHandle<SubPixelFunction>(env, obj)->getSubPixeling();
}


JNIEXPORT jint JNICALL Java_org_libpointing_TransferFunction_getHumanResolution
  (JNIEnv *env, jobject obj)
{
  return getHandle<SubPixelFunction>(env, obj)->getHumanResolution();
}


JNIEXPORT jint JNICALL Java_org_libpointing_TransferFunction_getCardinality
  (JNIEnv *env, jobject obj)
{
  int cardinality = 0, widgetSize = 0;
  getHandle<SubPixelFunction>(env, obj)->getCardinalitySize(&cardinality, &widgetSize);
  return cardinality;
}


JNIEXPORT jint JNICALL Java_org_libpointing_TransferFunction_getWidgetSize
  (JNIEnv *env, jobject obj)
{
  int cardinality = 0, widgetSize = 0;
  getHandle<SubPixelFunction>(env, obj)->getCardinalitySize(&cardinality, &widgetSize);
  return widgetSize;
}
