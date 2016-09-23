#ifndef FREQUENCY_ESTIMATOR_TEST_H
#define FREQUENCY_ESTIMATOR_TEST_H

#include <QObject>
#include <QtTest/QtTest>
#include <pointing/utils/TimeStamp.h>
#include <pointing/utils/FrequencyEstimator.h>

using namespace pointing;
using namespace std;

#define DELTA 0.0005

class FrequencyEstimatorTest : public QObject
{
    Q_OBJECT
    FrequencyEstimator fe;

private slots:

    // At the beginning
    void initTestCase()
    {
      double result = fe.estimatedFrequency();
      QCOMPARE(result, -1.);
    }

    void init()
    {
      fe.reset();
    }

    void frequency500()
    {
      TimeStamp::inttime t = TimeStamp::createAsInt();
      for (int i = 0; i < 25; i++)
      {
        float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        t += (2 + r / 2.) * TimeStamp::one_millisecond;
        fe.registerTimeStamp(t);
      }
      double result = fe.estimatedFrequency();
      QCOMPARE(result, 500.);
    }

    void changeFrequency()
    {
      TimeStamp::inttime t = TimeStamp::createAsInt();
      for (int i = 0; i < 15; i++)
      {
        float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        t += (4 + r / 2.) * TimeStamp::one_millisecond;
        fe.registerTimeStamp(t);
      }
      double result = fe.estimatedFrequency();
      QCOMPARE(result, 250.);

      for (int i = 0; i < 11; i++)
      {
        float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        t += (2 + r / 8.) * TimeStamp::one_millisecond;
        fe.registerTimeStamp(t);
      }
      result = fe.estimatedFrequency();
      QCOMPARE(result, 500.);
    }

    void frequency90()
    {
      TimeStamp::inttime t = TimeStamp::createAsInt();
      fe.registerTimeStamp(t);
      t += 8 * TimeStamp::one_millisecond;
      fe.registerTimeStamp(t);
      for (int i = 0; i < 10; i++)
      {
        t += 11 * TimeStamp::one_millisecond;
        fe.registerTimeStamp(t);
      }
      double result = fe.estimatedFrequency();
      QVERIFY(result < 90.9087 + DELTA && result > 90.9087 - DELTA);
    }

    void frequency117roundsTo125()
    {
      TimeStamp::inttime t = TimeStamp::createAsInt();
      fe.registerTimeStamp(t);
      t += 8 * TimeStamp::one_millisecond;
      fe.registerTimeStamp(t);
      for (int i = 0; i < 10; i++)
      {
        t += 8547 * TimeStamp::one_microsecond;
        fe.registerTimeStamp(t);
      }
      double result = fe.estimatedFrequency();
      QCOMPARE(result, 125.);
    }

    // At the end
    void cleanupTestCase()
    {
    }
};

#endif // FREQUENCY_ESTIMATOR_TEST_H

QTEST_MAIN(FrequencyEstimatorTest)
