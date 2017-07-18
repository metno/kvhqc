
#include "SimpleCorrectionsTest.hh"

#include "hqc/SimpleCorrections.hh"

#include "common/KvMetaDataBuffer.hh"
#include "common/KvalobsModelAccess.hh"
#include "common/test/CountingBuffer.hh"

#include <QComboBox>
#include <QLineEdit>

#define LOAD_DECL_ONLY
#include "common/test/load_18210_20130410.cc"

namespace /* anonymous */ {

struct TestSetup {
  EditAccess_p eda;
  ModelAccess_p mda;
  TestSetup();
};

TestSetup::TestSetup()
  : eda(std::make_shared<EditAccess>(FakeKvApp::app()->obsAccess()))
  , mda(std::make_shared<KvalobsModelAccess>(FakeKvApp::app()->obsAccess()->handler()))
{
}

} // anonymous

void TestSimpleCorrections::testGui()
{
  TestSetup ts;
  FakeKvApp::app()->clear();
  load_18210_20130410(*FakeKvApp::app());
  KvMetaDataBuffer::instance()->reload();

  std::unique_ptr<SimpleCorrections> sc(new SimpleCorrections(ts.eda, ts.mda, 0));

  QLineEdit* ui_station = sc->findChild<QLineEdit*>("textStation");
  QVERIFY(ui_station);

  QComboBox* ui_corrected = sc->findChild<QComboBox*>("comboCorrected");
  QVERIFY(ui_corrected);

  { const SensorTime st0(Sensor(18210, 211, 0, 0, 514), s2t("2013-04-02 00:00:00"));
    //orig=-0.8=corr ci=0111100000100010
    QVERIFY(st0.valid());
    
    sc->navigateTo(st0);
    QVERIFY2(sc->isEnabled(), "valid SensorTime should enable SimpleCorrections");
    
    QCOMPARE(QString::number(st0.sensor.stationId), ui_station->text());
    QCOMPARE(QString("18210 OSLO - HOVIN 100masl."), ui_station->toolTip());
    
    QVERIFY2(ui_corrected->isEnabled(), "valid SensorTime should enable corrected combo box");
    QVERIFY2(ui_corrected->isEditable(), "SensorTime for TA should enable editing combo box");
    QCOMPARE(ui_corrected->currentText(), QString("-0.8"));
  }

  { const SensorTime st1(Sensor(18210, 211, 0, 0, 514), s2t("2013-04-02 00:30:00"));
    //does not exist
    QVERIFY(st1.valid());

    sc->navigateTo(st1);
    QVERIFY2(sc->isEnabled(), "valid SensorTime should enable SimpleCorrections");

    QCOMPARE(QString::number(st1.sensor.stationId), ui_station->text());

    QVERIFY2(ui_corrected->isEnabled(), "valid SensorTime should enable corrected combo box");
    QVERIFY2(ui_corrected->isEditable(), "SensorTime for TA should enable editing combo box");
    QVERIFY(ui_corrected->currentText().isEmpty());

    const float newC = -2.0;
    const QString newCtext("-2.0");

    CountingBuffer_p counter(new CountingBuffer(st1.sensor, TimeSpan(st1.time, st1.time)));
    counter->syncRequest(ts.eda);
    QCOMPARE(counter->countComplete, size_t(1));
    QVERIFY(not counter->get(st1));
    counter->zero();

    { ts.eda->newVersion();
      ObsUpdate_pv updates;
    
      ObsUpdate_p up = ts.eda->createUpdate(st1);
      QVERIFY((bool)up);
    
      up->setCorrected(newC);
      updates.push_back(up);
      QVERIFY(ts.eda->storeUpdates(updates));
    }
    
    QVERIFY((bool)counter->get(st1));
    QCOMPARE(ui_corrected->currentText(), newCtext);

    ts.eda->undoVersion();

    QVERIFY(not counter->get(st1));
    QVERIFY(ui_corrected->currentText().isEmpty());

    ts.eda->redoVersion();
    QVERIFY((bool)counter->get(st1));
    QCOMPARE(ui_corrected->currentText(), newCtext);
  }

  { const SensorTime st2(Sensor(18210, 211, 0, 0, 514), s2t("2013-04-02 01:30:00"));
    //does not exist
    QVERIFY(st2.valid());

    sc->navigateTo(st2);
    QVERIFY2(sc->isEnabled(), "valid SensorTime should enable SimpleCorrections");

    QCOMPARE(QString::number(st2.sensor.stationId), ui_station->text());

    QVERIFY2(ui_corrected->isEnabled(), "valid SensorTime should enable corrected combo box");
    QVERIFY2(ui_corrected->isEditable(), "SensorTime for TA should enable editing combo box");
    QVERIFY(ui_corrected->currentText().isEmpty());

    const float newC = -3.0;
    const QString newCtext("-3.0");

    CountingBuffer_p counter(new CountingBuffer(st2.sensor, TimeSpan(st2.time, st2.time)));
    counter->syncRequest(ts.eda);
    QCOMPARE(counter->countComplete, size_t(1));
    QVERIFY(not counter->get(st2));
    counter->zero();

    ui_corrected->setEditText(newCtext);
    QTest::keyClick(ui_corrected, Qt::Key_Return);
    
    QVERIFY((bool)counter->get(st2));
    QCOMPARE(ui_corrected->currentText(), newCtext);

    ts.eda->undoVersion();

    QVERIFY(not counter->get(st2));
    QVERIFY(ui_corrected->currentText().isEmpty());

    ts.eda->redoVersion();
    QVERIFY((bool)counter->get(st2));
    QCOMPARE(ui_corrected->currentText(), newCtext);
  }
}
