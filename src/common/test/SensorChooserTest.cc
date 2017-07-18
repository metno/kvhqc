
#include "SensorChooserTest.hh"

#include "common/KvMetaDataBuffer.hh"
#include "common/SensorChooser.hh"

#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>

#define LOAD_DECL_ONLY
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

    const SensorTime st(Sensor(stationid, 61, 0, 0, 308), s2t("2014-10-01 06:00:00"));
    sc.setSensorTime(st);

    station.clear();
    QTest::keyClicks(&station, QString::number(stationid));

    QCOMPARE(sc.getSensor().stationId, stationid);
    QVERIFY2(param.isEnabled(), "known station should enable parameter selection");
    QVERIFY2(sc.isValid(), "known station should have at least one parameter/typeid/level/sensor");

    const int idxP81 = param.findData(81);
    QVERIFY2(idxP81 >= 0, "parameter 81 is in obs_pgm and should appear in param QComboBox");
    param.setCurrentIndex(idxP81);
    QCOMPARE(sc.getSensor().paramId, 81);

    QCOMPARE(sc.getSensor().typeId, st.sensor.typeId);
  }
}
