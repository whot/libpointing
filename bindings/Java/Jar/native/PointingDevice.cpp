/* -*- mode: c++ -*-
 *
 *  Bindings/Java/Jar/native/PointingDevice.cpp--
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

#include "org_libpointing_PointingDevice.h"
#include <pointing/pointing.h>
#include "handle.h"

using namespace pointing;

JavaVM *jvm;

void pointingCallback(void *context, TimeStamp::inttime timestamp,
                      int input_dx, int input_dy, int buttons)
{
  JNIEnv *env;
  jvm->AttachCurrentThread((void **)&env, NULL);

  jobject obj = (jobject)context;

  jmethodID mid;
  jclass handlerClass = env->FindClass("org/libpointing/PointingDevice");

  if (handlerClass == NULL) {
    std::cerr << "Error : cannot find class PointingDevice" << std::endl;
  }
  mid = env->GetMethodID(handlerClass, "callback", "(JIII)V");
  if (mid == NULL) {
    std::cerr << "Error : cannot find method callback" << std::endl;
  }
  else {
    jlong ts = timestamp;
    jint jinput_dx = input_dx;
    jint jinput_dy = input_dy;
    jint jbuttons = buttons;

    env->CallVoidMethod(obj, mid, ts, jinput_dx, jinput_dy, jbuttons);
  }
}

JNIEXPORT jlong JNICALL Java_org_libpointing_PointingDevice_initPointingDevice
(JNIEnv *env, jobject jobj, jstring uriStr)
{
  jobject obj = env->NewGlobalRef(jobj);
  env->GetJavaVM(&jvm);
  const char *uri = env->GetStringUTFChars(uriStr, 0);
  PointingDevice *device = PointingDevice::create(uri);
  device->setPointingCallback(pointingCallback, (void *)obj);
  env->ReleaseStringUTFChars(uriStr, uri);
  return (jlong)device;
}

JNIEXPORT void JNICALL Java_org_libpointing_PointingDevice_releasePointingDevice
(JNIEnv *env, jobject obj)
{
  delete getHandle<PointingDevice>(env, obj);
}

JNIEXPORT jstring JNICALL Java_org_libpointing_PointingDevice_getURI
(JNIEnv *env, jobject obj)
{
  PointingDevice *p = getHandle<PointingDevice>(env, obj);
  return env->NewStringUTF(p->getURI().asString().c_str());
}

JNIEXPORT jdouble JNICALL Java_org_libpointing_PointingDevice_getResolution
(JNIEnv *env, jobject obj)
{
  return getHandle<PointingDevice>(env, obj)->getResolution();
}

JNIEXPORT jdouble JNICALL Java_org_libpointing_PointingDevice_getUpdateFrequency
(JNIEnv *env, jobject obj)
{
  return getHandle<PointingDevice>(env, obj)->getUpdateFrequency();
}

JNIEXPORT jboolean JNICALL Java_org_libpointing_PointingDevice_isActive
(JNIEnv *env, jobject obj)
{
  return getHandle<PointingDevice>(env, obj)->isActive();
}

JNIEXPORT jint JNICALL Java_org_libpointing_PointingDevice_getVendorID
(JNIEnv *env, jobject obj)
{
  return getHandle<PointingDevice>(env, obj)->getVendorID();
}

JNIEXPORT jstring JNICALL Java_org_libpointing_PointingDevice_getVendor
(JNIEnv *env, jobject obj)
{
  return env->NewStringUTF(getHandle<PointingDevice>(env, obj)->getVendor().c_str());
}

JNIEXPORT jint JNICALL Java_org_libpointing_PointingDevice_getProductID
(JNIEnv *env, jobject obj)
{
  return getHandle<PointingDevice>(env, obj)->getProductID();
}

JNIEXPORT jstring JNICALL Java_org_libpointing_PointingDevice_getProduct
(JNIEnv *env, jobject obj)
{
  return env->NewStringUTF(getHandle<PointingDevice>(env, obj)->getProduct().c_str());
}

JNIEXPORT void JNICALL Java_org_libpointing_PointingDevice_setDebugLevel
(JNIEnv *env, jobject obj, jint debugLevel)
{
  getHandle<PointingDevice>(env, obj)->setDebugLevel(debugLevel);
}

JNIEXPORT void JNICALL Java_org_libpointing_PointingDevice_idle
(JNIEnv *, jclass, jint idleTime)
{
  PointingDevice::idle(idleTime);
}
