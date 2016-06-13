/* -*- mode: c++ -*-
 *
 * apps/controlMouse/FunctionChanger.cpp --
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

#include "FunctionChanger.h"

// When this mode is turned on all other pointing devices
// are blocked since update is performed by pointingCallback
#define DIRECT_UPDATE_MODE 1

using namespace pointing;
using namespace std;

CGEventRef FunctionChanger::callback_aQuartzEventHasBeenReceived(CGEventTapProxy /* proxy */, CGEventType type, CGEventRef event, void *refcon) {
  FunctionChanger *self = (FunctionChanger *)refcon;
  return self->modifyQuartzEvent(event, type);
}

void FunctionChanger::updateRectList()
{
  DisplayDeviceManager *manager = DisplayDeviceManager::get();
  dispRects.clear();
  for (DisplayDescriptorIterator it = manager->begin(); it != manager->end(); it++)
  {
    DisplayDeviceDescriptor desc = *it;
    DisplayDevice *device = DisplayDevice::create(desc.devURI);
    DisplayDevice::Bounds bounds = device->getBounds();
    CGRect rect;
    rect.origin.x = bounds.origin.x;
    rect.origin.y = bounds.origin.y;
    rect.size.height = bounds.size.height;
    rect.size.width = bounds.size.width;
    dispRects.push_back(rect);
    delete device;
  }
}

void FunctionChanger::callback(void *context, const DisplayDeviceDescriptor &, bool)
{
  FunctionChanger *self = (FunctionChanger *)context;
  self->updateRectList();
}

CGFloat distanceBetween2(CGPoint p1, CGPoint p2)
{
  CGFloat xDist = p2.x - p1.x;
  CGFloat yDist = p2.y - p1.y;
  return sqrt((xDist * xDist) + (yDist * yDist));
}

CGPoint getClosest(CGRect rect, CGPoint point)
{
  CGPoint closest = rect.origin;
  if (rect.origin.x + rect.size.width < point.x)
    closest.x += rect.size.width; // point is far right of us
  else if (point.x > rect.origin.x)
    closest.x = point.x; // point above or below us
  if (rect.origin.y + rect.size.height < point.y)
    closest.y += rect.size.height; // point is far below us
  else if (point.y > rect.origin.y)
    closest.y = point.y; // point is straight left or right
  return closest;
}

void FunctionChanger::updateToClosest(CGPoint newPosition)
{
  CGPoint closest;
  closest.x = 0, closest.y = 0;
  CGFloat minDist = distanceBetween2(newPosition, closest);
  for (list<CGRect>::iterator it = dispRects.begin(); it != dispRects.end(); it++)
  {
    CGRect rect = *it;
    if (CGRectContainsPoint(rect, newPosition))
    {
      curPosition = newPosition;
      return;
    }
    CGPoint newClosest = getClosest(rect, newPosition);

    CGFloat dist = distanceBetween2(newPosition, newClosest);
    if (dist < minDist)
    {
      closest = newClosest;
      minDist = dist;
    }
  }
  curPosition = closest;
}

FunctionChanger::FunctionChanger(TransferFunction *function):annihilates(true),source(NULL)
{
  func = function;
  curPosition = getCurrentCursorPosition();
  eventTap = CGEventTapCreate(kCGHIDEventTap, kCGHeadInsertEventTap, kCGEventTapOptionDefault, CGEventMaskBit(kCGEventMouseMoved) | CGEventMaskBit(kCGEventLeftMouseDragged) | CGEventMaskBit(kCGEventRightMouseDragged), callback_aQuartzEventHasBeenReceived, this);
  CFRunLoopSourceRef runLoopSourceForTap = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);
  // Add the tap to the current run loop
  CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSourceForTap, kCFRunLoopDefaultMode);
  CGEventTapEnable(eventTap, true);
  DisplayDeviceManager *manager = DisplayDeviceManager::get();
  updateRectList();
  manager->addDeviceUpdateCallback(callback, this);
}

void FunctionChanger::setDxDy(double dx, double dy, CGMouseButton buttons)
{
  double dpx = 0, dpy = 0;
  func->applyd(dx, dy, &dpx, &dpy, TimeStamp::createAsInt());

  CGPoint newPosition;
  newPosition.x = curPosition.x + dpx;
  newPosition.y = curPosition.y + dpy;

  CGWarpMouseCursorPosition(curPosition);

  updateToClosest(newPosition);

#if DIRECT_UPDATE_MODE == 0
  updateOnScreen(buttons);
#endif
}

CGEventSourceRef FunctionChanger::eventSource()
{
  if(!source)
  {
    source = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);
    CGEventSourceSetUserData(source, (int64_t)this);
  }
  return source;
}

void FunctionChanger::updateOnScreen(CGMouseButton buttons)
{
  // Warp the on-screen cursor
  CGWarpMouseCursorPosition(curPosition);

  CGMouseButton pressedButtons = buttons;

  CGEventType eventType = kCGEventMouseMoved;

  if(pressedButtons == 1<<0) {
    eventType = kCGEventLeftMouseDragged;
  } else if(pressedButtons == 1<<1) {
    eventType = kCGEventRightMouseDragged;
  }

  // Generate an event
  CGEventRef newEvent = CGEventCreateMouseEvent(eventSource(), eventType, curPosition, pressedButtons);
  CGEventPost(kCGHIDEventTap, newEvent);
  CFRelease(newEvent);
}

CGEventRef FunctionChanger::modifyQuartzEvent(CGEventRef event, CGEventType type)
{
  if (annihilates)
  {
#if DIRECT_UPDATE_MODE == 1
    CGWarpMouseCursorPosition(curPosition);
#endif

    if(type == kCGEventLeftMouseDragged || type == kCGEventRightMouseDragged || type == kCGEventMouseMoved) {
      CGEventSetLocation(event, curPosition);
#if DIRECT_UPDATE_MODE == 0
      CGEventSourceRef sourceOfEvent = CGEventCreateSourceFromEvent(event);
      if(sourceOfEvent && CGEventSourceGetUserData(sourceOfEvent) == (int64_t)this) {
          // This is our mouse move event, we let it pass through
          CFRelease(sourceOfEvent);
          return event;
      }
      CFRelease(sourceOfEvent);
      // This isn't our event. We stop it.
      return NULL;
      // return NULL does work: when you return NULL, the cursor is still moving but mousemove events don't get sent anymore (try hovering window traffic lights)
#endif
    }
  }
  return event;
}

CGPoint FunctionChanger::getCurrentCursorPosition()
{
  CGPoint point;
  CGEventRef event = CGEventCreate(NULL);
  point = CGEventGetLocation(event);
  CFRelease(event);
  return point;
}


void FunctionChanger::setAnnihilates(bool value)
{
  annihilates = value;
}
