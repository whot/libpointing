#include "winEchoMouse.h"
#include <windows.h>
#include <sstream>
#include <fstream>
#include <pointing/transferfunctions/windows/winSystemPointerAcceleration.h>

#define           N_ACCS_WIN     11
#define           DEFAULT_POS    5
#define           SLEEP_TIME     10
static float win_accs[N_ACCS_WIN] = {-5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5};

using namespace std;

winEchoMouse::winEchoMouse(std::string folder):EchoMouse(folder)
{
    n_accs = N_ACCS_WIN;
    default_pos = DEFAULT_POS;
    accs = win_accs;
    sleep_time = SLEEP_TIME;
    //cout << b.size.width << " " << b.size.height << endl;

    // Open the device using the VID, PID,
    // and optionally the Serial number.
    handle = hid_open(0x46d, 0xc05a, NULL);
    if (handle == NULL) {
      std::cerr << "Pb opening EchoMouseDev" << std::endl;
    }
}

int winEchoMouse::Send(unsigned char x, unsigned char y, unsigned char scroll, bool leftClick, bool middleClick, bool rightClick)
{
  // http://www.computer-engineering.org/ps2mouse/
  unsigned char buttons = 0x8;
  if (leftClick) buttons |= 0x1;
  if (rightClick) buttons |= 0x2;
  if (middleClick) buttons |= 0x4;

  // Output reports
  #if 0
    unsigned char data[4] = {x, y, scroll, 0};
    data[3] = buttons;
    int res = hid_write(handle, data, sizeof(data));
  // Feature reports
  #else
    // Always keep the first byte equal to 0!
    unsigned char data[5] = {0, x, y, scroll, 0};
    data[4] = buttons;
    int res= hid_send_feature_report(handle, data, sizeof(data));
  #endif
  return res;
}

void winEchoMouse::sleep(int milliseconds)
{
    Sleep(milliseconds);
}

void winEchoMouse::moveCursorForNPoints(int nPoints)
{
    Send(nPoints, 0, 0, false, false, false);
}

void winEchoMouse::writeFunctionForAcc(int acc_ind, map<int, float> &mapVal)
{
    static char posAlp[N_ACCS_WIN][9] = { "First", "Second", "Third", "Fourth", "Fifth",
                                          "Sixth", "Seventh", "Eighth", "Nineth", "Tenth", "Eleventh"};
    ofstream file;
    stringstream ss;
    ss << folderName << "/f" << acc_ind + 1 << ".dat";
    file.open(ss.str().c_str(), ios::out | ios::trunc);

    file << "# " << posAlp[acc_ind]
         << " position of the slider with the value "
         << accs[acc_ind] << endl << endl;

    file << "max-counts: 127" << endl;
    file << "# counts: pixels" << endl;

    for (map<int, float>::const_iterator it = mapVal.begin(); it != mapVal.end(); it++)
    {
        file << it->first << ": " << it->second << endl;
    }
    file.close();
}

void winEchoMouse::writeTransferFunctions(int nCounts)
{
    writeConfigDict();
    winSystemPointerAcceleration acc;
    for (int i = 0; i < n_accs; i++)
    {
        map<int, float> mapVal;
        mapVal[0] = 0;
        acc.set(accs[i], true);
        for (int pp = 1; pp < nCounts; pp++)
        {
            float px = getPxForDPoints(pp);
            cout << "acc_i: " << i << " pp: " << pp << " px: " << px << endl;
            mapVal[pp] = px;
        }
        writeFunctionForAcc(i, mapVal);
    }
    acc.set(accs[default_pos], true);
}

string winEchoMouse::getSystemVersion()
{
    return "Windows_8";
}

winEchoMouse::~winEchoMouse()
{
}

