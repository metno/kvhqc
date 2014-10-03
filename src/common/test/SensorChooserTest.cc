
#include "SensorChooserTest.hh"

#include "common/KvMetaDataBuffer.hh"
#include "common/SensorChooser.hh"

#include "load_31850_20121130.cc"

void TestSensorChooser::testGui()
{
  load_31850_20121130(*FakeKvApp::app());
  KvMetaDataBuffer::instance()->reload();

  QLineEdit station;
  QComboBox param, type, level;
  QSpinBox sensorNr;

  SensorChooser sc(&station, &param, &type, &level, &sensorNr);
  QVERIFY2(not sc.isValid(), "empty station is not valid");

  { const int stationid = 83880;
    QVERIFY(not KvMetaDataBuffer::instance()->isKnownStation(stationid));

    QTest::keyClicks(&station, QString::number(stationid), Qt::NoModifier, 100);

    QCOMPARE(sc.getSensor().stationId, stationid);
    QVERIFY2(not param.isEnabled(), "unknown station should disable parameter selection");
    QVERIFY(not sc.isValid());
  }

  { const int stationid = 31850;
    QVERIFY(KvMetaDataBuffer::instance()->isKnownStation(stationid));

    station.clear();
    QTest::keyClicks(&station, QString::number(stationid), Qt::NoModifier, 100);

    QCOMPARE(sc.getSensor().stationId, stationid);
    QVERIFY2(param.isEnabled(), "known station should enable parameter selection");
    QVERIFY(not sc.isValid());
  }
}
