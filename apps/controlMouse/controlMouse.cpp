/* -*- mode: c++ -*-
 *
 * apps/controlMouse/controlMouse.cpp --
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

#include <pointing/pointing.h>
#include <pointing/transferfunctions/SubPixelFunction.h>
#include <pointing/output/DisplayDeviceManager.h>
#include "FunctionChanger.h"

using namespace pointing;
using namespace std;

FunctionChanger *changer = 0;

void pointingCallback(void *, TimeStamp::inttime, int input_dx, int input_dy, int buttons) {
  if (!(input_dx || input_dy)) return;
  changer->setDxDy(input_dx, input_dy, (CGMouseButton)buttons);
}

void callback(void *, const DisplayDeviceDescriptor &, bool)
{
  // TODO
}

int main()
{
  const int cardinality = 20000;
  const int widgetSize = 283;

  PointingDevice *input = PointingDevice::create("any:?vendor=0x46d");
  DisplayDevice *output = DisplayDevice::create("any:?debugLevel=1");

  DisplayDeviceManager *manager = DisplayDeviceManager::get();
  manager->addDeviceUpdateCallback(callback, NULL);

  URI uri("osx:?debugLevel=2");
  URI subUri = "subpixel:?debugLevel=2";

  SubPixelFunction *func = new SubPixelFunction(subUri, uri, input, output) ;

  func->setCardinalitySize(cardinality, widgetSize);
  changer = new FunctionChanger(func);

  // To receive events from PointingDevice object, a callback function must be set.
  input->setPointingCallback(pointingCallback);
  while (1)
    PointingDevice::idle(100); // milliseconds

  delete input ;
  delete output ;
  delete func ;

  return 0;
}
