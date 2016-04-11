/* -*- mode: c++ -*-
 *
 * pointing/input/windows/winHIDDeviceDispatcher.cpp --
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
/* See file LICENSE in the top-directory of the project. */


#include <pointing/input/windows/winHIDDeviceDispatcher.h>
#include <pointing/input/windows/winPointingDeviceManager.h>

#include <windows.h>

namespace pointing {

  void winHIDDeviceDispatcher::addPointingDevice(HANDLE h, winPointingDevice *device)
  {
    std::map<HANDLE, PointingList>::iterator it = callbackMap.find(h);
    if (it == callbackMap.end())
    {
      PointingList list;
      list.push_back(device);
      callbackMap[h] = list;
    }
    else
    {
      it->second.push_back(device);
    }
    if (existingDevices.count(h) > 0)
    {
      device->setActive(h, true);
    }
  }

  void winHIDDeviceDispatcher::removePointingDevice(HANDLE h, winPointingDevice *device)
  {
    std::map<HANDLE, PointingList>::iterator it = callbackMap.find(h);
    if (it != callbackMap.end())
    {
      it->second.remove(device);
    }
  }

  void winHIDDeviceDispatcher::processMessage(MSG *msg)
  {
    TranslateMessage(msg);
    DispatchMessage(msg);
  }

  DWORD WINAPI winHIDDeviceDispatcher::Loop(LPVOID lpvThreadParam)
  {
    winHIDDeviceDispatcher* self = (winHIDDeviceDispatcher*) lpvThreadParam;
    self->msghwnd_=self->rawInputInit();
    MSG msg;

    // This while loop makes sure that all connected devices
    // are registered, only then we allow running the main loop
    // It is necessary to avoid the case when a specified device is not
    // found in the dispatcher at the beginning of the program.
    while (PeekMessage(&msg, self->msghwnd_, 0, 0, PM_REMOVE))
    {
      self->processMessage(&msg);
      if (msg.message != WM_INPUT_DEVICE_CHANGE)
        break;
    }
    self->run = THREAD_RUNNING;

    while (1)
      if ( GetMessage(&msg, self->msghwnd_, 0, 0) )
        self->processMessage(&msg);

    self->run=THREAD_HALTED;
    return 0;
  }

  HWND winHIDDeviceDispatcher::rawInputInit()
  {
    HWND tempHwnd = 0;
    WNDCLASSEX w;
    memset(&w, 0, sizeof(w));
    w.cbSize = sizeof(w);
    w.lpfnWndProc = (WNDPROC)winHIDDeviceDispatcher::rawInputProc;
    w.lpszClassName = L"MyServiceWindowClass";
    ATOM atom = ::RegisterClassEx(&w);
    if (!atom){ throw std::runtime_error("Unable to register a new windows class for message processing"); }
    tempHwnd=CreateWindow( w.lpszClassName,
                           L"",
                           WS_BORDER | WS_CAPTION,
                           0, 0, 0, 0,
                           HWND_MESSAGE ,
                           NULL,
                           NULL,
                           NULL);
    if(!tempHwnd){ throw std::runtime_error("Unable to create a message window"); }
    SetWindowLongPtr(tempHwnd, GWLP_USERDATA, (LONG_PTR)this);

    RAWINPUTDEVICE Rid[1];
    Rid[0].usUsagePage = 0x01;
    Rid[0].usUsage = 0x02;
    Rid[0].dwFlags = RIDEV_INPUTSINK |  RIDEV_DEVNOTIFY ;   // adds HID mouse
    Rid[0].hwndTarget = tempHwnd;

    //tempHwnd = GetConsoleWindow();

    if (RegisterRawInputDevices(Rid, 1, sizeof(Rid[0])) == FALSE) {
      throw std::runtime_error("Unable to register this window to rawinput.");
    }

    return tempHwnd;
  }

  void winHIDDeviceDispatcher::activateDevice(HANDLE h, bool active)
  {
    for (std::map<HANDLE, PointingList>::iterator it = callbackMap.begin(); it != callbackMap.end(); it++)
    {
      PointingList pointingList = it->second;
      for (PointingList::iterator it = pointingList.begin(); it != pointingList.end(); it++)
      {
        winPointingDevice *dev = *it;
        if (dev->callback != NULL)
          dev->setActive(h, active);
      }
    }
  }

  // Static function that process the rawinput events and let the others processed by the default processor.
  // hwnd field stores the window handler, uMsg is the event type, wParam and lParam are additional
  // parameters.
  LONG APIENTRY winHIDDeviceDispatcher::rawInputProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
  {
    // Check if we have received a rawinput event
    // This can be either insertion/removal of a device or a message from one of them.
    if(uMsg==WM_INPUT_DEVICE_CHANGE)
    {
      // We have stored in the GWL_USERDATA a pointer to the winPointingDevice. This object
      // is needed to route the event to the user provided callback and context.
      winHIDDeviceDispatcher* self=(winHIDDeviceDispatcher*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

      switch(wParam)
      {
      case GIDC_ARRIVAL:
      {
        self->existingDevices.insert(((HANDLE)lParam));
        RID_DEVICE_INFO deviceinfo;
        self->manager->registerMouseDevice((HANDLE)lParam, deviceinfo);
        self->activateDevice((HANDLE)lParam, true);
        break;
      }
      case GIDC_REMOVAL:
        self->existingDevices.erase(((HANDLE)lParam));
        self->manager->unregisterMouseDevice((HANDLE)lParam);
        self->activateDevice((HANDLE)lParam, false);
        break;
      }

      return 0;
    }
    else if(uMsg==WM_INPUT)
    {
      UINT dwSize = 0;

      // We have stored in the GWL_USERDATA a pointer to the winPointingDevice. This object
      // is needed to route the event to the user provided callback and context.
      winHIDDeviceDispatcher* self=(winHIDDeviceDispatcher*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

      // Retreive the raw input data...
      GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize,
                      sizeof(RAWINPUTHEADER));
      LPBYTE lpb = new BYTE[dwSize];
      if (lpb == NULL)
        return 0;

      if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize,
                          sizeof(RAWINPUTHEADER)) != dwSize)
        throw std::runtime_error("GetRawInputData does not return correct size !\n");
      //ON_ERROR("GetRawInputData does not return correct size !\n");

      RAWINPUT* raw = (RAWINPUT*)lpb;
      if (raw->header.dwType == RIM_TYPEMOUSE)
      {
        //std::cout << "Input frame  from: " << std::hex << raw->header.hDevice << std::endl;
        if(self->callbackMap.find(raw->header.hDevice) != self->callbackMap.end())
        {
          TimeStamp::inttime now = TimeStamp::createAsInt();
          PointingList pointingList = self->callbackMap[raw->header.hDevice];
          for (PointingList::iterator it = pointingList.begin(); it != pointingList.end(); it++)
          {
            winPointingDevice *dev = *it;

            // To prevent calling the callback function for simple touchs
            // we verify that there is a button clicked or a displacement
            // Otherwise for some touchpads the callback is called even if you
            // hold your finger on the touchpad.
            if (raw->data.mouse.usButtonFlags || raw->data.mouse.lLastX || raw->data.mouse.lLastY)
            {
              if(raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN)
                dev->buttons |= 1 << 0;
              if(raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP)
                dev->buttons &= ~(1 << 0);

              if(raw->data.mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN)
                dev->buttons |= 1 << 1;
              if(raw->data.mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP)
                dev->buttons &= ~(1 << 1);

              if(raw->data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN)
                dev->buttons |= 1 << 2;
              if(raw->data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP)
                dev->buttons &= ~(1 << 2);

              dev->registerTimestamp(now);
              if (dev->callback != NULL)
                dev->callback(dev->callback_context, now, raw->data.mouse.lLastX, raw->data.mouse.lLastY, dev->buttons);
            }
          }
        }
      }
      delete[] lpb;
      return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
  }

  winHIDDeviceDispatcher::~winHIDDeviceDispatcher(void) {
    run = THREAD_TERMINATING;
    while(run != THREAD_HALTED){Sleep(100);}
    //DestroyWindow(msghwnd_);
  }

  winHIDDeviceDispatcher::winHIDDeviceDispatcher(winPointingDeviceManager *manager)
    :manager(manager)
  {
    run = THREAD_UNDEFINED;
    hThreads[0]=CreateThread(NULL, NULL, Loop, LPVOID(this), 0, &dwThreadId);
    while(run==THREAD_UNDEFINED){Sleep(10);}
  }
}
