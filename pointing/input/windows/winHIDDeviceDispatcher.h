/* -*- mode: c++ -*-
 *
 * pointing/input/windows/winHIDDeviceDispatcher.h --
 *
 * Initial software
 * Authors: Izzat Mukhanov
 * Copyright © INRIA
 *
 */

#ifndef WINHIDDEVICEHANDLER_H
#define WINHIDDEVICEHANDLER_H

#include <windows.h>
#include <map>
#include <list>
#include <set>
#include <pointing/input/windows/winPointingDevice.h>

namespace pointing {

  class winPointingDeviceManager;

  /**
   * @brief The winHIDDeviceDispatcher class is used to handle all the input devices,
   * their messages and their addition or removal.
   * winPointingDevice must subscribe to this class object in order to receive mouse events.
   * So, all the events are managed here and dispatched to a specific winPointingDevice
   * Another thread is used for message loop.
   */
  class winHIDDeviceDispatcher
  {
    winPointingDeviceManager *manager;

    typedef std::list<winPointingDevice *> PointingList;

    void activateDevice(HANDLE h, bool active);

    std::map<HANDLE, PointingList> callbackMap;

    std::set<HANDLE> existingDevices;

    typedef enum
    {
        THREAD_UNDEFINED=0,
        THREAD_RUNNING,
        THREAD_TERMINATING,
        THREAD_HALTED
    } ThreadState;

    ThreadState run; // for the Loop thread
    HANDLE hThreads[1];
    DWORD dwThreadId;
    static DWORD WINAPI Loop(LPVOID lpvThreadParam);
    void processMessage(MSG *msg);
    HWND msghwnd_;

    static LONG APIENTRY rawInputProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    HWND rawInputInit();
	
	// True if the mouse has moved
	bool relativeDisplacement(const PRAWMOUSE pmouse, winPointingDevice *dev, int *dx, int *dy);

  public:

    winHIDDeviceDispatcher(winPointingDeviceManager *manager);
    ~winHIDDeviceDispatcher(void);

    void addPointingDevice(HANDLE h, winPointingDevice *device);
    void removePointingDevice(HANDLE h, winPointingDevice *device);
  };

}

#endif // WINHIDDEVICEHANDLER_H
