/* -*- mode: c++ -*-
 *
 * Bindings/Java/Jar/native/DisplayDevice.cpp --
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
#include "org_libpointing_DisplayDevice.h"
#include "handle.h"

#include <pointing/pointing.h>

using namespace pointing;

JNIEXPORT jlong JNICALL Java_org_libpointing_DisplayDevice_initDisplayDevice
(JNIEnv *env, jobject, jstring uriStr)
{
  const char *uri = env->GetStringUTFChars(uriStr, 0);
  DisplayDevice *device = DisplayDevice::create(uri);
  env->ReleaseStringUTFChars(uriStr, uri);
  return (jlong)device;
}

JNIEXPORT void JNICALL Java_org_libpointing_DisplayDevice_releaseDisplayDevice
(JNIEnv *env, jobject obj)
{
  delete getHandle<DisplayDevice>(env, obj);
}

JNIEXPORT jstring JNICALL Java_org_libpointing_DisplayDevice_getURI
(JNIEnv * env, jobject obj)
{
  DisplayDevice *p = getHandle<DisplayDevice>(env, obj);
  return env->NewStringUTF(p->getURI().asString().c_str());
}

JNIEXPORT jdouble JNICALL Java_org_libpointing_DisplayDevice_getResolution
(JNIEnv *env, jobject obj)
{
  return getHandle<DisplayDevice>(env, obj)->getResolution();
}

JNIEXPORT jdouble JNICALL Java_org_libpointing_DisplayDevice_getRefreshRate
(JNIEnv *env, jobject obj)
{
  return getHandle<DisplayDevice>(env, obj)->getRefreshRate();
}

JNIEXPORT jobject JNICALL Java_org_libpointing_DisplayDevice_getBounds
(JNIEnv *env, jobject obj)
{
  jclass handlerClass = env->FindClass("java/awt/Rectangle");
  jmethodID mid = env->GetMethodID(handlerClass, "setRect", "(DDDD)V");
  DisplayDevice::Bounds b = getHandle<DisplayDevice>(env, obj)->getBounds();
  return env->NewObject(handlerClass, mid, b.origin.x, b.origin.y, b.size.width, b.size.height);
}

JNIEXPORT jobject JNICALL Java_org_libpointing_DisplayDevice_getSize
(JNIEnv *env, jobject obj)
{
  jclass handlerClass = env->FindClass("java/awt/Dimension");
  jmethodID mid = env->GetMethodID(handlerClass, "setSize", "(DD)V");
  DisplayDevice::Size s = getHandle<DisplayDevice>(env, obj)->getSize();
  return env->NewObject(handlerClass, mid, s.width, s.height);
}

JNIEXPORT void JNICALL Java_org_libpointing_DisplayDevice_setDebugLevel
(JNIEnv *env , jobject obj, jint debugLevel)
{
  getHandle<DisplayDevice>(env, obj)->setDebugLevel(debugLevel);
}
