#ifndef POINTING_DEVICE_TEST_H
#define POINTING_DEVICE_TEST_H

#include <QObject>
#include <QtTest/QtTest>
#include <pointing/pointing.h>

using namespace pointing;
using namespace std;

class PointingDeviceTest : public QObject
{
    Q_OBJECT

private slots:

    void emptyUri()
    {
      PointingDevice *input = PointingDevice::create();
      URI result = input->getURI(false, true);
      QCOMPARE(result.scheme, string("any"));
      int debugLevel = -1;
      URI::getQueryArg(result.query, "debugLevel", &debugLevel);
      QCOMPARE(debugLevel, 1);
      delete input;
    }

    void anyUri()
    {
      string uri = "any:?vendor=1";
      PointingDevice *input = PointingDevice::create(uri);
      URI result = input->getURI(false, true);
      QCOMPARE(result.asString(), uri);
      result = input->getURI(false, false);
      //std::cerr << result.asString() << std::endl;
      QCOMPARE(result.asString(), uri);
      result = input->getURI(true, false);
      int debugLevel = -1;
      URI::getQueryArg(result.query, "debugLevel", &debugLevel);
      QCOMPARE(debugLevel, 0);
      delete input;
    }

    void cpiHz()
    {
      std::string uri = "any:?cpi=888&hz=555";
      PointingDevice *input = PointingDevice::create(uri);
      double cpi = input->getResolution();
      QCOMPARE(cpi, 888.);
      double hz = input->getUpdateFrequency();
      QCOMPARE(hz, 555.);
      delete input;
    }

    void productVendorIDs()
    {
      std::string uri = "any:?vendor=1&product=2";
      PointingDevice *input = PointingDevice::create(uri);
      int vendorID = input->getVendorID();
      QCOMPARE(vendorID, 1);
      int productID = input->getProductID();
      QCOMPARE(productID, 2);
      delete input;
    }

    void isNotActive()
    {
      std::string uri = "any:?vendor=1&product=2";
      PointingDevice *input = PointingDevice::create(uri);
      QCOMPARE(input->isActive(), false);
      delete input;
    }
};

#endif // POINTING_DEVICE_TEST_H

QTEST_MAIN(PointingDeviceTest)
