/* -*- mode: c++ -*-
 *
 * apps/echoMouse/main.cpp --
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

#include <iostream>
#ifdef __APPLE__
#include "osxEchoMouse.h"
#endif
#ifdef WIN32
#include "winEchoMouse.h"
#endif
#ifdef __linux__
#include "xorgEchoMouse.h"
#endif

using namespace std;

int main(int argc, char** argv)
{
#ifdef __APPLE__
    string devPath = "/dev/cu.usbmodemHIDG1", outputFolder = "elcapitan";
    if (argc < 2)
    {
        cout << "Usage: " << argv[0] << " [pathToDevice [outputFolder]]" << endl;
        cout << "Device Path by default: " << devPath << " Output folder by default: " << outputFolder << endl;
    }
    if (argc > 1)
        devPath = argv[1];
    if (argc > 2)
        devPath = argv[2];

    osxEchoMouse echoMouse(devPath, outputFolder);
    //EchoMouse *mouse = EchoMouse::create(devPath, outputFolder);
    echoMouse.writeTransferFunctions(128);
#endif

#ifdef WIN32
    winEchoMouse echoMouse("Test");
    echoMouse.writeTransferFunctions(128);
#endif

#ifdef __linux__
    string devPath = "/dev/tty.usbmodem14211", outputFolder = "test";
    if (argc < 2)
    {
        cout << "Usage: " << argv[0] << " [pathToDevice [outputFolder]]" << endl;
        cout << "Device Path by default: " << devPath << " Output folder by default: " << outputFolder << endl;
    }
    if (argc > 1)
        devPath = argv[1];
    if (argc > 2)
        devPath = argv[2];

    xorgEchoMouse echoMouse(devPath, "Linux");
    echoMouse.writeTransferFunctions(128);
#endif

    return 0;
}

