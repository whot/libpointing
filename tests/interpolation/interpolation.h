#ifndef INTERPOLATION_TEST_H
#define INTERPOLATION_TEST_H

#include <QObject>
#include <QtTest/QtTest>
#include <pointing/transferfunctions/Interpolation.h>
#include <pointing/utils/FileUtils.h>

using namespace pointing;
using namespace std;

#define DELTA 0.0005

class InterpolationTest : public QObject
{
    Q_OBJECT
    Interpolation *func;

private slots:

    // At the beginning
    void initTestCase()
    {
        URI uri("interp:");
        func = new Interpolation(uri, 0, 0);
    }

    // Each time a function is called
    void init()
    {

    }

    void ZeroPoints()
    {
        int dxP, dyP;
        func->clearState();
        func->applyi(0, 0, &dxP, &dyP);
        QCOMPARE(dxP, 0);
        QCOMPARE(dyP, 0);
    }

    void OutOfBoundPoints()
    {
        func->loadTableStr("max-counts: 1\n"
                           "0: 0\n"
                           "1: 0.2\n");
        int dxP, dyP;
        func->applyi(6, 0, &dxP, &dyP);
        QCOMPARE(dxP, 1);
        func->applyi(4, 0, &dxP, &dyP);
        QCOMPARE(dxP, 1);
        func->applyi(100, 0, &dxP, &dyP);
        QCOMPARE(dxP, 20);
    }

    void ApplyDoublePrecision()
    {
        func->loadTableStr("max-counts: 1\n"
                           "0: 0\n"
                           "1: 0.2\n");
        double dxP, dyP;
        func->clearState();
        func->applyd(4, 0, &dxP, &dyP);
        QVERIFY(dxP < 0.8 + DELTA && dxP > 0.8 - DELTA);
    }

    void HugeValues()
    {
        func->loadTableStr("max-counts: 1\n"
                           "0: 0\n"
                           "1: 0.2\n");
        int dxP, dyP;
        func->clearState();
        func->applyi(1000, 0, &dxP, &dyP);
        QCOMPARE(dxP, 200);
    }

    void InterpolateBetween2()
    {
        func->loadTableStr("max-counts: 3\n"
                           "0: 0\n"
                           "1: 2\n"
                           "3: 6\n");
        int dxP, dyP;
        func->applyi(2, 0, &dxP, &dyP);
        QCOMPARE(dxP, 4);
    }

    void InterpolateFromFar()
    {
        func->loadTableStr("max-counts: 11\n"
                           "0: 0\n"
                           "1: 1\n"
                           "5: 10\n"
                           "11: 22\n");
        int dxP, dyP;
        func->clearState();
        func->applyi(2, 0, &dxP, &dyP);
        QCOMPARE(dxP, 3);
        func->clearState();
        func->applyi(9, 0, &dxP, &dyP);
        QCOMPARE(dxP, 18);
    }

    void NegativeValues()
    {
        func->loadTableStr("max-counts: 1\n"
                           "0: 0\n"
                           "1: 0.2\n");
        int dxP, dyP;
        // This should give 0.2 which is floored to 0 not to 1
        func->applyi(-1, 0, &dxP, &dyP);
        QCOMPARE(dxP, 0);
    }

    void replacedOSXURI()
    {
      TransferFunction *f = TransferFunction::create("osx:?setting=1.5", 0, 0);
      URI uri = f->getURI();
      QCOMPARE(uri.scheme, std::string("interp"));
      double setting = 0.;
      URI::getQueryArg(uri.query, "f", &setting);
      QCOMPARE(setting, 1.5);
      delete f;
    }

    void replacedWindowsURI()
    {
      TransferFunction *f = TransferFunction::create("windows:", 0, 0);
      URI uri = f->getURI();
      QCOMPARE(uri.scheme, std::string("interp"));
      int slider = -1;
      URI::getQueryArg(uri.query, "f", &slider);
      QCOMPARE(slider, 0);
      std::string modulePath = moduleHeadersPath();
      QCOMPARE(uri.path, modulePath + "/pointing-echomouse/windows/epp");
      delete f;
    }

    void sliderWindowsEpp()
    {
      // For Windows transfer function with epp 1 point displacement
      // should start giving 1 px displacement at slider = 4
      // So, slider = [-5, 3] should return 0 for 1 point displacement
      TransferFunction *f = TransferFunction::create("windows:?slider=3", 0, 0);
      int dxP, dyP;
      f->applyi(1, 0, &dxP, &dyP);
      QCOMPARE(dxP, 0);
      delete f;
      f = TransferFunction::create("windows:?slider=4", 0, 0);
      f->applyi(1, 0, &dxP, &dyP);
      QCOMPARE(dxP, 1);
      delete f;
    }

    void defaultSettingIfWrongArgument()
    {
      // Slider = 7 option does not exist, so the default setting should be applied
      TransferFunction *f = TransferFunction::create("windows:?slider=7", 0, 0);
      int dxP, dyP;
      f->applyi(100, 0, &dxP, &dyP);
      QCOMPARE(dxP, 253);
      delete f;
    }

    void normalizedFunction()
    {
      PointingDevice *i = PointingDevice::create("dummy:?cpi=800");
      DisplayDevice *o = DisplayDevice::create("dummy:?ppi=96");
      TransferFunction *f = TransferFunction::create("windows:?normalize=true", i, o);
      int dxP, dyP;
      f->applyi(10, 0, &dxP, &dyP);
      QCOMPARE(dxP, 7);
      delete f; delete i; delete o;
    }

    void windowsNoEpp()
    {
      TransferFunction *f = TransferFunction::create("windows:?f=f9&epp=false", 0, 0);
      int dxP, dyP;
      f->applyi(102, 0, &dxP, &dyP);
      QCOMPARE(dxP, 255);
      delete f;
    }

    void MoreThanOneByteInputValues()
    {
      // For devices where dx and dy more than 1 byte
      // We need to make sure that output values are correct
      TransferFunction *f = TransferFunction::create("windows:?slider=2&epp=true", 0, 0);
      int dxP, dyP;
      f->applyi(13398, 44530, &dxP, &dyP);
      QCOMPARE(dxP, 26796);
      QCOMPARE(dyP, 89060);
      delete f;
    }

    void replacedWindowsWithArgsURI()
    {
      TransferFunction *f = TransferFunction::create("windows:8?epp=false&slider=3", 0, 0);
      URI uri = f->getURI();
      QCOMPARE(uri.scheme, std::string("interp"));
      int slider = -1;
      URI::getQueryArg(uri.query, "f", &slider);
      QCOMPARE(slider, 3);
      std::string modulePath = moduleHeadersPath();
      QCOMPARE(uri.path, modulePath + "/pointing-echomouse/windows/no-epp");
      delete f;
    }

    // At the end
    void cleanupTestCase()
    {
        delete func;
    }
};

#endif // INTERPOLATION_TEST_H

QTEST_MAIN(InterpolationTest)
