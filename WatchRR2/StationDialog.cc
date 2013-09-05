
#include "StationDialog.hh"

#include "AnalyseRR24.hh"
#include "KvMetaDataBuffer.hh"
#include "RedistTableModel.hh"
#include "TimeRangeControl.hh"

#include "ui_watchrr_station.h"

#include <kvalobs/kvObsPgm.h>
#include <kvcpp/KvApp.h>

#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.StationDialog"
#include "HqcLogging.hh"

static const int MIN_DAYS = 7, MAX_DAYS = 120;

StationDialog::StationDialog(const Sensor& sensor, const TimeRange& time, QDialog* parent)
  : QDialog(parent)
  , mSensor(sensor)
{
  init();

  ui->editStation->setText(QString::number(mSensor.stationId));
  ui->dateFrom->setDateTime(timeutil::to_QDateTime(time.t0()));
  ui->dateTo  ->setDateTime(timeutil::to_QDateTime(time.t1()));

  onUpdateType();
}

StationDialog::StationDialog(QDialog* parent)
  : QDialog(parent)
  , mSensor(-1, kvalobs::PARAMID_RR_24, 0, 0, -1)
{
  init();
  onUpdateType();
}

void StationDialog::init()
{
  METLIBS_LOG_SCOPE();
  ui.reset(new Ui::DialogStation);
  ui->setupUi(this);
  
  ui->labelStationInfo->setText("");

  QDateTime today(QDate::currentDate(), QTime(6, 0));
  ui->dateFrom->setDateTime(today.addDays(-4*MIN_DAYS));
  ui->dateTo  ->setDateTime(today);

  mTimeControl = new TimeRangeControl(this);
  mTimeControl->setMinimumGap(4*24);
  mTimeControl->install(ui->dateFrom, ui->dateTo);
}

StationDialog::~StationDialog()
{
  METLIBS_LOG_SCOPE();
}

void StationDialog::onUpdateType()
{
  const bool ok = check();
  ui->buttonOK->setEnabled(ok);

  QString typeText;
  if (mSensor.typeId >= 0)
    typeText = QString::number(mSensor.typeId);
  else
    typeText = tr("invalid station or date range");
  ui->labelTypeInfo->setText(typeText);
}

bool StationDialog::acceptThisObsPgm(const kvalobs::kvObsPgm& op) const
{
  return (op.paramID() == kvalobs::PARAMID_RR_24
      and (op.typeID() == 302 or op.typeID() == 402)
      and (op.kl06() or op.kl07() or op.collector()));
}

bool StationDialog::check()
{
  bool ok = false;
  const int stationId = ui->editStation->text().toInt(&ok);
  if (not ok) {
    ui->labelStationInfo->setText(tr("Cannot read station number"));
    return false;
  }
  if (stationId < 60 or stationId > 100000) {
    ui->labelStationInfo->setText(tr("Invalid station number"));
    return false;
  }

  mSensor.stationId = stationId;
  const std::list<kvalobs::kvObsPgm>& obs_pgm = KvMetaDataBuffer::instance()->findObsPgm(mSensor.stationId);
  if (obs_pgm.empty()) {
    ui->labelStationInfo->setText(tr("problem loading obs_pgm"));
    return false;
  }

  const timeutil::ptime tFrom = timeutil::from_QDateTime(QDateTime(ui->dateFrom->dateTime()));
  const timeutil::ptime tTo   = timeutil::from_QDateTime(QDateTime(ui->dateTo  ->dateTime()));
  int typeFrom = -1, typeTo = -1;
  timeutil::ptime tFromEnd, tToStart;
  BOOST_FOREACH (const kvalobs::kvObsPgm& op, obs_pgm) {
    if (acceptThisObsPgm(op)) {
      const timeutil::ptime oFrom = timeutil::from_miTime(op.fromtime()), oTo = timeutil::from_miTime(op.totime());
      if (oFrom <= tFrom and (oTo.is_not_a_date_time() or tFrom <= oTo)) {
        typeFrom = op.typeID();
        tFromEnd = oTo;
      }
      if (oFrom <= tTo and (oTo.is_not_a_date_time() or tTo <= oTo)) {
        typeTo = op.typeID();
        tToStart = oFrom;
      }
    }
  }

  if (typeFrom < 0 or typeTo < 0) {
    ui->labelStationInfo->setText(tr("could not find typeid"));
    return false;
  }

  if (typeFrom != typeTo) {
    ui->labelStationInfo->setText(tr("typeid different at start and end"));
    return false;
  } else {
    mSensor.typeId = typeFrom; // same as typeTo
  }
  ui->labelStationInfo->setText("");
  return true;
}

bool StationDialog::valid() const
{
  return mSensor.stationId >= 60 and mSensor.stationId <= 99999
      and mSensor.typeId >= 1;
}

void StationDialog::onButtonOk()
{
  METLIBS_LOG_SCOPE();
  if (valid())
    accept();
}

TimeRange StationDialog::selectedTime() const
{
  const timeutil::ptime tFrom = timeutil::from_QDateTime(QDateTime(ui->dateFrom->dateTime()));
  const timeutil::ptime tTo   = timeutil::from_QDateTime(QDateTime(ui->dateTo  ->dateTime()));
  return TimeRange(tFrom, tTo);
}
