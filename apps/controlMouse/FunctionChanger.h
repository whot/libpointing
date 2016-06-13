/* -*- mode: c++ -*-
 *
 * apps/controlMouse/FunctionChanger.h --
 *
 * Initial software
 * Authors: Jonathan Aceituno, Izzat Mukhanov
 * Copyright © Inria
 *
 * http://libpointing.org/
 *
 * This software may be used and distributed according to the terms of
 * the GNU General Public License version 2 or any later version.
 *
 */

#ifndef FUNCTIONCHANGER_H
#define FUNCTIONCHANGER_H

#include <ApplicationServices/ApplicationServices.h>
#include <pointing/pointing.h>
#include <pointing/output/DisplayDeviceManager.h>
#include <list>

class FunctionChanger
{
  pointing::TransferFunction *func;
  bool annihilates;
  CGPoint curPosition;
  CFMachPortRef eventTap;
  CGEventSourceRef source;

  void updateRectList();
  std::list<CGRect> dispRects;

  CGEventRef modifyQuartzEvent(CGEventRef event, CGEventType type);
  CGPoint getCurrentCursorPosition();
  void updateOnScreen(CGMouseButton buttons);
  static CGEventRef callback_aQuartzEventHasBeenReceived(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon);
  CGEventSourceRef eventSource();
  static void callback(void *context, const pointing::DisplayDeviceDescriptor &, bool wasAdded);

  void updateToClosest(CGPoint newPosition);

public:

  FunctionChanger(pointing::TransferFunction *function);

  void setDxDy(double dx, double dy, CGMouseButton /*buttons*/);

  void setAnnihilates(bool value);
};

#endif // FUNCTIONCHANGER_H
