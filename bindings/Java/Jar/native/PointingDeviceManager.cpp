/* -*- mode: c++ -*-
 *
 *  Bindings/Java/Jar/native/PointingDeviceManager.cpp--
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
#include "org_libpointing_PointingDeviceManager.h"

#include <pointing/input/PointingDevice.h>
#include <pointing/input/PointingDeviceManager.h>

using namespace pointing;

JavaVM *machine;
jobject ob;

jobject objectForDescriptor(JNIEnv *env, const PointingDeviceDescriptor &desc)
{
  jclass descClass = env->FindClass("org/libpointing/PointingDeviceDescriptor");
  if (descClass == NULL)
    std::cerr << "Error : cannot find class PointingDeviceDescriptor" << std::endl;
  jmethodID ctor = env->GetMethodID(descClass, "<init>", "(Ljava/lang/String;Ljava/lang/String;II)V");
  if (ctor == NULL)
    std::cerr << "Error : cannot find constructor" << std::endl;
  
  jobject feature = env->NewObject(descClass, ctor, env->NewStringUTF(desc.devURI.c_str()),
                                   env->NewStringUTF(desc.name.c_str()), desc.vendorID, desc.productID);
  return feature;
}

void updateDeviceCallback(void *, const PointingDeviceDescriptor &desc, bool wasAdded)
{ 
  JNIEnv *env;
  machine->AttachCurrentThread((void **)&env, NULL);

  jmethodID mid;
  jclass handlerClass = env->FindClass("org/libpointing/PointingDeviceManager");

  if (handlerClass == NULL)
    std::cerr << "Error : cannot find class PointingDeviceManager" << std::endl;
  jobject feature = objectForDescriptor(env, desc);

  if (wasAdded)
    mid = env->GetMethodID(handlerClass, "deviceAdded", "(Lorg/libpointing/PointingDeviceDescriptor;)V");
  else
    mid = env->GetMethodID(handlerClass, "deviceRemoved", "(Lorg/libpointing/PointingDeviceDescriptor;)V");
  
  if (mid == NULL)
    std::cerr << "Error : cannot find method callback" << std::endl;
  
  env->CallVoidMethod(ob, mid, feature);
}

JNIEXPORT void JNICALL Java_org_libpointing_PointingDeviceManager_setCallbacks
(JNIEnv *env, jobject jobj)
{
  env->GetJavaVM(&machine);
  ob = env->NewGlobalRef(jobj);
  PointingDeviceManager *manager = PointingDeviceManager::get();
  manager->addDeviceUpdateCallback(updateDeviceCallback, (void*)(env));
}

JNIEXPORT jobjectArray JNICALL Java_org_libpointing_PointingDeviceManager_getDeviceList
(JNIEnv * env, jobject) {
  PointingDeviceManager *manager = PointingDeviceManager::get();
  int size = manager->size();
  jobjectArray ret;
  ret = (jobjectArray) env->NewObjectArray(size, env->FindClass("org/libpointing/PointingDeviceDescriptor"), NULL);
  int i = 0;
  for (PointingDescriptorIterator it = manager->begin(); it != manager->end() && i < size; it++, i++)
  {
    PointingDeviceDescriptor desc = *it;
    env->SetObjectArrayElement(ret, i, objectForDescriptor(env, desc));
  }
  return ret;
}

JNIEXPORT jint JNICALL Java_org_libpointing_PointingDeviceManager_size
(JNIEnv *, jobject)
{
  PointingDeviceManager *manager = PointingDeviceManager::get();
  return manager->size();
}
