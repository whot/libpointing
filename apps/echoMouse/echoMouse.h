/* -*- mode: c++ -*-
 *
 * apps/echoMouse/echoMouse.h --
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

#ifndef ECHOMOUSE_H
#define ECHOMOUSE_H

#include <pointing/pointing.h>
#include <map>

using namespace pointing;

class EchoMouse
{
protected:
    PointingDevice *input = 0;
    DisplayDevice *output = 0;

    std::string folderName;

    int n_accs = 0;
    int default_pos = 0;
    float *accs = 0;
    int sleep_time = 100;

    virtual void moveCursorForNPoints(int nPoints) = 0;
    virtual std::string getSystemVersion() = 0;
    virtual void sleep(int milliseconds) = 0;

    float getPxForDPoints(int dPoints);
    virtual void writeConfigDict();

    EchoMouse(std::string folder);

public:
    virtual ~EchoMouse();
    virtual void writeTransferFunctions(int nCounts) = 0;
};

#endif // ECHOMOUSE_H
