#ifndef WINECHOMOUSE_H
#define WINECHOMOUSE_H

#include "echoMouse.h"
#include "hidapi.h"

class winEchoMouse : public EchoMouse
{
    hid_device *handle;
    int Send(unsigned char x, unsigned char y, unsigned char scroll, bool leftClick, bool middleClick, bool rightClick);

    void moveCursorForNPoints(int nPoints);

    std::string getSystemVersion();
    void sleep(int milliseconds);

    void writeFunctionForAcc(int acc_ind, std::map<int, float> &mapVal);

public:
    winEchoMouse(std::string folder);
    void writeTransferFunctions(int nCounts);
    ~winEchoMouse();
};

#endif // WINECHOMOUSE_H
