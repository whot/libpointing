#ifndef OSXECHOMOUSE_H
#define OSXECHOMOUSE_H

#include "echoMouse.h"

class osxEchoMouse : public EchoMouse
{
    int fd; // File descriptor

    virtual void moveCursorForNPoints(int nPoints);

    std::string getSystemVersion();
    void sleep(int milliseconds);

    void writeFunctionForAcc(int acc_ind, std::map<int, float> &mapVal);

public:
    void writeTransferFunctions(int nCounts);
    osxEchoMouse(std::string devName, std::string folder);
    ~osxEchoMouse();
};

#endif // OSXECHOMOUSE_H
