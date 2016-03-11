/* -*- mode: c++ -*-
 *
 * Bindings/Java/Jar/native/TimeStamp.cpp --
 *
 * Initial software
 * Authors: Gery Casiez
 * Copyright Inria
 *
 * http://libpointing.org/
 *
 * This software may be used and distributed according to the terms of
 * the GNU General Public License version 2 or any later version.
 *
 */

#include <jni.h>
#include <iostream>
#include "org_libpointing_TimeStamp.h"

#include <pointing/utils/TimeStamp.h>

using namespace pointing;

JNIEXPORT jlong JNICALL Java_org_libpointing_TimeStamp_getTimestamp
  (JNIEnv *, jobject) {
    return TimeStamp::createAsInt();
}


