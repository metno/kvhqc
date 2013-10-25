
#include "StationDialog.hh"

#include "RedistTableModel.hh"
#include "common/AnalyseRR24.hh"
#include "common/KvMetaDataBuffer.hh"
#include "common/StationIdCompletion.hh"
#include "common/TypeIdModel.hh"
#include "common/gui/TimeRangeControl.hh"

#include "ui_watchrr_station.h"

#include <kvalobs/kvObsPgm.h>

#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.StationDialog"
#include "util/HqcLogging.hh"

static const int MIN_DAYS = 7, MAX_DAYS = 120;

StationDialog::StationDialog(const Sensor& sensor, const TimeRange& time, QDialog* parent)
  : QDialog(parent)
  , mSensor(sensor)
{
  init();

  ui->editStation->setText(QString::number(mSensor.stationId));
  ui->dateFrom->setDateTime(timeutil::to_QDateTime(time.t0()));
  ui->dateTo  ->setDateTime(timeutil::to_QDateTime(time.t1()));

  onEditStation();
}

StationDialog::StationDialog(QDialog* parent)
  : QDialog(parent)
  , mSensor(-1, kvalobs::PARAMID_RR_24, 0, 0, -1)
{
  init();

  onEditStation();
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

  Helpers::installStationIdCompleter(this, ui->editStation);
}

StationDialog::~StationDialog()
{
  METLIBS_LOG_SCOPE();
}

void StationDialog::onEditStation()
{
  checkStation();
  updateTypeList();
  enableOk();
}

void StationDialog::onEditTime()
{
  updateTypeList();
  enableOk();
}

void StationDialog::onSelectType(int)
{
  checkType();
  enableOk();
}

bool StationDialog::checkStation()
{
  bool numberOk = false;
  const int stationId = ui->editStation->text().toInt(&numberOk);
  if (not numberOk) {
    ui->labelStationInfo->setText(tr("Cannot read station number"));
  } else if (stationId < 60 or stationId > 100000) {
    ui->labelStationInfo->setText(tr("Invalid station number"));
  } else {
    mSensor.stationId = stationId;
    QString name = "?";
    try {
        const kvalobs::kvStation& station = KvMetaDataBuffer::instance()->findStation(stationId);
        name = QString::fromStdString(station.name());
    } catch (std::exception& e) {
        HQC_LOG_WARN("Error in station lookup: " << e.what());
    }
    ui->labelStationInfo->setText(name);
    return true;
  } 
  mSensor.stationId = 0;
  return false;
}

bool StationDialog::checkType()
{
  mSensor.typeId = 0;
  if (mTypesModel.get()) {
    const int idx = ui->comboType->currentIndex();
    if (idx >= 0 and idx < (int)mTypesModel->values().size())
      mSensor.typeId = mTypesModel->values().at(idx);
  }
  return mSensor.typeId != 0;
}

int StationDialog::acceptThisObsPgm(const kvalobs::kvObsPgm& op) const
{
  if (not (op.kl06() or op.kl07() or op.collector()))
    return 0;
  if (op.paramID() == kvalobs::PARAMID_RR_24)
    return op.typeID();
  if (Helpers::aggregatedParameter(op.paramID(), kvalobs::PARAMID_RR_24))
    return -op.typeID();
  return 0;
}

void StationDialog::updateTypeList()
{
  std::set<int> typeIdSet;
  if (mSensor.stationId > 0) {
    const std::list<kvalobs::kvObsPgm>& obs_pgm = KvMetaDataBuffer::instance()->findObsPgm(mSensor.stationId);
    if (obs_pgm.empty()) {
      ui->labelStationInfo->setText(tr("problem loading obs_pgm"));
    } else {
      const TimeRange st = selectedTime();
      BOOST_FOREACH (const kvalobs::kvObsPgm& op, obs_pgm) {
        if (st.intersection(TimeRange(op.fromtime(), op.totime())).undef())
          continue;
        const int t = acceptThisObsPgm(op);
        if (t != 0)
          typeIdSet.insert(t);
      }
      if (typeIdSet.empty())
        ui->labelStationInfo->setText(tr("could not find typeid"));
    }
  }
  const std::vector<int> newTypeIds(typeIdSet.begin(), typeIdSet.end());
  
  if (not mTypesModel.get() or newTypeIds != mTypesModel->values()) {
    mTypesModel.reset(new TypeIdModel(newTypeIds));
    ui->comboType->setModel(mTypesModel.get());
    if (not typeIdSet.empty()) {
      ui->comboType->setCurrentIndex(0);
      mSensor.typeId = mTypesModel->values().front();
    } else {
      mSensor.typeId = 0;
    }
  }
}

bool StationDialog::valid() const
{
  return mSensor.stationId >= 60 and mSensor.stationId < 100000
      and mSensor.typeId != 0 and selectedTime().closed();
}

void StationDialog::enableOk()
{
  ui->buttonOK->setEnabled(valid());
}

TimeRange StationDialog::selectedTime() const
{
  const timeutil::ptime tFrom = timeutil::from_QDateTime(QDateTime(ui->dateFrom->dateTime()));
  const timeutil::ptime tTo   = timeutil::from_QDateTime(QDateTime(ui->dateTo  ->dateTime()));
  return TimeRange(tFrom, tTo);
}
