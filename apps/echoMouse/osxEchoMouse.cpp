#include "osxEchoMouse.h"
#include "serial.h"
#include <sstream>
#include <fstream>
#include <ApplicationServices/ApplicationServices.h>
#include <pointing/transferfunctions/osx/osxSystemPointerAcceleration.h>
#include <sys/utsname.h>

using namespace std;

#define           N_ACCS_MAC     10
#define           DEFAULT_POS    3
#define           SLEEP_TIME     66
static float mac_accs[N_ACCS_MAC] = {0, 0.125, 0.5, 0.6875, 0.875, 1, 1.5, 2, 2.5, 3};

osxEchoMouse::osxEchoMouse(std::string devName, std::string folder):EchoMouse(folder)
{
    n_accs = N_ACCS_MAC;
    default_pos = DEFAULT_POS;
    accs = mac_accs;
    sleep_time = SLEEP_TIME;
    //cout << b.size.width << " " << b.size.height << endl;
    fd = openPort(devName);
    setOptions(fd);
}

void osxEchoMouse::sleep(int milliseconds)
{
    usleep(1000*milliseconds);
}

void osxEchoMouse::moveCursorForNPoints(int nPoints)
{
    //stringstream ss;
    //ss << "0_0_" << nPoints << "_0" << endl;
    writeByte(fd, nPoints);
}

void osxEchoMouse::writeFunctionForAcc(int acc_ind, map<int, float> &mapVal)
{
    static char posAlp[N_ACCS_MAC][8] = { "First", "Second", "Third", "Fourth", "Fifth",
                                          "Sixth", "Seventh", "Eighth", "Nineth", "Tenth"};
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

void osxEchoMouse::writeTransferFunctions(int nCounts)
{
    writeConfigDict();
    osxSystemPointerAcceleration acc;
    for (int i = 0; i < n_accs; i++)
    {
        map<int, float> mapVal;
        mapVal[0] = 0;
        acc.set(accs[i], "mouse");
        for (int pp = 1; pp < nCounts; pp++)
        {
            float px = getPxForDPoints(pp);
            cout << "acc_i: " << i << " pp: " << pp << " px: " << px << endl;
            mapVal[pp] = px;
        }
        writeFunctionForAcc(i, mapVal);
    }
    acc.set(accs[default_pos], "mouse");
}

/**
 * @brief getSystemVersion equivalent to "uname -rs"
 * @return string corresponding to the version of the system
 */
string osxEchoMouse::getSystemVersion()
{
    struct utsname sysinfo;
    uname(&sysinfo);
    return string(sysinfo.sysname) + "_" + string(sysinfo.release);
}

osxEchoMouse::~osxEchoMouse()
{
    closePort(fd);
}
