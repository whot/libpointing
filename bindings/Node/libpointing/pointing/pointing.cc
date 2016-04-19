/* -*- mode: c++ -*-
 * 
 * pointing/pointing.cc
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

#include <node.h>
#include "ntransferfunction.h"
#include "npointingdevicemanager.h"
#include "ndisplaydevicemanager.h"
#include "nsystempointeracceleration.h"
#include "npointingcursor.h"

#ifdef __APPLE__
#include <uv.h>
#include <CoreFoundation/CoreFoundation.h>

uv_idle_t idler;

void perform_while_idle(uv_idle_t* handle)
{
  CFRunLoopRun();
  // uv_idle_stop(handle);
}

#endif

NAN_MODULE_INIT(InitAll)
{
#ifdef __APPLE__
  uv_idle_init(uv_default_loop(), &idler);
  uv_idle_start(&idler, (uv_idle_cb)perform_while_idle);
#endif
  NPointingDevice::Init(target);
  NDisplayDevice::Init(target);
  NTransferFunction::Init(target);
  NPointingDeviceManager::Init(target);
  NDisplayDeviceManager::Init(target);
  NSystemPointerAcceleration::Init(target);
  NPointingCursor::Init(target);
}

NODE_MODULE(pointing, InitAll)