/* -*- mode: c++ -*-
 *
 *  Bindings/Java/Jar/native/DisplayDeviceManager.cpp--
 *
 * Initial software
 * Authors: Izzat Mukhanov
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
#include "org_libpointing_DisplayDeviceManager.h"

#include <pointing/output/DisplayDevice.h>
#include <pointing/output/DisplayDeviceManager.h>

using namespace pointing;

JavaVM *ddm_machine;
jobject ddm_ob;

jobject objectForDescriptor(JNIEnv *env, const DisplayDeviceDescriptor &desc)
{
  jclass descClass = env->FindClass("org/libpointing/DisplayDeviceDescriptor");
  if (descClass == NULL)
    std::cerr << "Error : cannot find class DisplayDeviceDescriptor" << std::endl;
  jmethodID ctor = env->GetMethodID(descClass, "<init>", "(Ljava/lang/String;Ljava/lang/String;II)V");
  if (ctor == NULL)
    std::cerr << "Error : cannot find constructor" << std::endl;
  
  jobject feature = env->NewObject(descClass, ctor, env->NewStringUTF(desc.devURI.c_str()),
                                   env->NewStringUTF(desc.name.c_str()), desc.width, desc.height);
  return feature;
}

// Fixme: Callback is not working
void updateDeviceCallback(void *, const DisplayDeviceDescriptor &desc, bool wasAdded)
{
  JNIEnv *env;
  ddm_machine->AttachCurrentThread((void **)&env, NULL);


  jmethodID mid;
  jclass handlerClass = env->FindClass("org/libpointing/DisplayDeviceManager");

  if (handlerClass == NULL)
    std::cerr << "Error : cannot find class DisplayDeviceManager" << std::endl;
  jobject feature = objectForDescriptor(env, desc);

  if (wasAdded)
    mid = env->GetMethodID(handlerClass, "deviceAdded", "(Lorg/libpointing/DisplayDeviceDescriptor;)V");
  else
    mid = env->GetMethodID(handlerClass, "deviceRemoved", "(Lorg/libpointing/DisplayDeviceDescriptor;)V");
  
  if (mid == NULL)
    std::cerr << "Error : cannot find method callback" << std::endl;
  
  env->CallVoidMethod(ddm_ob, mid, feature);
}

JNIEXPORT void JNICALL Java_org_libpointing_DisplayDeviceManager_setCallbacks
(JNIEnv *env, jobject jobj)
{
  env->GetJavaVM(&ddm_machine);
  ddm_ob = env->NewGlobalRef(jobj);
  DisplayDeviceManager *manager = DisplayDeviceManager::get();
  manager->addDeviceUpdateCallback(updateDeviceCallback, (void*)(env));
}

JNIEXPORT jobjectArray JNICALL Java_org_libpointing_DisplayDeviceManager_getDeviceList
(JNIEnv * env, jobject) {
  DisplayDeviceManager *manager = DisplayDeviceManager::get();
  int size = manager->size();
  jobjectArray ret;
  ret = (jobjectArray) env->NewObjectArray(size, env->FindClass("org/libpointing/DisplayDeviceDescriptor"), NULL);
  int i = 0;
  for (DisplayDescriptorIterator it = manager->begin(); it != manager->end() && i < size; it++, i++)
  {
    DisplayDeviceDescriptor desc = *it;
    env->SetObjectArrayElement(ret, i, objectForDescriptor(env, desc));
  }
  return ret;
}

JNIEXPORT jint JNICALL Java_org_libpointing_DisplayDeviceManager_size
(JNIEnv *, jobject)
{
  DisplayDeviceManager *manager = DisplayDeviceManager::get();
  return manager->size();
}
