/* -*- mode: c++ -*-
 *
 * apps/echoMouse/echoMouse.cpp --
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

#include <sstream>
#include <iterator>
#include <fstream>
#include <pointing/utils/PointingCursor.h>
#include "echoMouse.h"
#ifdef __APPLE__
#include "osxEchoMouse.h"
#endif

#ifdef WIN32
#include "winEchoMouse.h"
#endif

using namespace std;

#define EPS 5 // This is needed to stop the loop, because sometimes when the cursor at the boundary
              // of the display it doesn't move further and outputs the value like 1679.12

#define N_MAX   60

float EchoMouse::getPxForDPoints(int dPoints)
{
    int width = output->getBounds().size.width;
    int height = output->getBounds().size.height;
    PointingCursor::setPosition(0, height / 2);
    sleep(sleep_time);
    double px = 0, py = 0, prev_px = 0;
    int n;
    for (n = 0; px < width - EPS; n++)
    {
        prev_px = px;
        //cout << dPoints << endl;
        moveCursorForNPoints(dPoints);
        sleep(sleep_time);
        PointingCursor::getPosition(&px, &py);
        if (n > N_MAX)
            break;
        //cout << "diff: " << px - prev_px << endl;
    }
    return prev_px / (n - 1);
}

void EchoMouse::writeConfigDict()
{
    ofstream file;
    string fileName = folderName + "/config.dict";
    std::cout << fileName << std::endl;
    file.open(fileName.c_str(), ios::out | ios::trunc);

    /* ---------- System Info ---------- */
    file << "system: " << getSystemVersion()
         << endl << endl;

    /* ---------- Input and Output URIs ---------- */
    file << "# Created with libpointing" << endl;
    file << "libpointing-input: "
         << input->getURI(true, true).asString()
         << endl << endl;
    file << "# Obtained with libpointing" << endl;
    file << "libpointing-output: "
         << output->getURI(true).asString()
         << endl << endl;

    /* ---------- Functions info ---------- */
    file << "functions: ";
    stringstream sFunc;
    stringstream sAlias;
    for (int i = 0; i < n_accs; i++)
    {
        if(i != 0)
        {
            sFunc << ",";
            sAlias << ",";
        }
        sFunc << "f" << i + 1;
        sAlias << accs[i];
    }
    file << sFunc.str() << endl;
    file << "function-aliases: "
         << sAlias.str() << endl << endl;

    file << "default-function: f"
         << default_pos + 1 << endl;
    file.close();
}

EchoMouse::EchoMouse(std::string folder):folderName(folder)
{
    input = PointingDevice::create("dummy:?cpi=400&hz=125");
    output = DisplayDevice::create("any:");
#ifdef WIN32
    // FIXME
    output = DisplayDevice::create("dummy:?bx=0&by=0&bw=1680&bh=1050&w=474.133&h=296.333&hz=60");
#endif
    cout << output->getURI(true).asString() << endl;
    DisplayDevice::Bounds b;
    b = output->getBounds();
    //cout << b.size.width << " " << b.size.height << endl;
}

EchoMouse::~EchoMouse()
{
    delete output;
    delete input;
}
