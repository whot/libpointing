#ifndef INTERPOLATION_TEST_H
#define INTERPOLATION_TEST_H

#include <QObject>
#include <QtTest/QtTest>
#include <pointing/transferfunctions/Interpolation.h>
#include <pointing/utils/FileUtils.h>

using namespace pointing;
using namespace std;

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
