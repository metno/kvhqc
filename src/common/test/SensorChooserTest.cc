
#include "SensorChooserTest.hh"

#include "common/KvMetaDataBuffer.hh"
#include "common/SensorChooser.hh"

#include "load_18700_20141001.cc"

void TestSensorChooser::testGui()
{
  FakeKvApp::app()->clear();
  load_18700_20141001(*FakeKvApp::app());
  KvMetaDataBuffer::instance()->reload();

  QLineEdit station;
  QComboBox param, type, level;
  QSpinBox sensorNr;

  SensorChooser sc(&station, &param, &type, &level, &sensorNr,
      0, false /*completion disturbs test*/);
  QVERIFY2(not sc.isValid(), "empty station is not valid");

  { const int stationid = 18210;
    QVERIFY(not KvMetaDataBuffer::instance()->isKnownStation(stationid));

    QTest::keyClicks(&station, QString::number(stationid));

    QCOMPARE(sc.getSensor().stationId, stationid);
    QVERIFY2(not param.isEnabled(), "unknown station should disable parameter selection");
    QVERIFY(not sc.isValid());
  }

  { const int stationid = 18700;
    QVERIFY(KvMetaDataBuffer::instance()->isKnownStation(stationid));

    station.clear();
    QTest::keyClicks(&station, QString::number(stationid));

    QCOMPARE(sc.getSensor().stationId, stationid);
    QVERIFY2(param.isEnabled(), "known station should enable parameter selection");
    QVERIFY2(sc.isValid(), "known station should have at least one parameter/typeid/level/sensor");
  }
}
