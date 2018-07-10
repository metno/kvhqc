/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2012-2018 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: kvalobs-dev@met.no

  This file is part of HQC

  HQC is free software; you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  HQC is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General Public License
  along with HQC; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include "StationDialog.hh"

#include "RedistTableModel.hh"
#include "AnalyseRR24.hh"
#include "common/KvHelpers.hh"
#include "common/KvMetaDataBuffer.hh"
#include "common/ObsPgmRequest.hh"
#include "common/StationIdCompletion.hh"
#include "common/TypeIdModel.hh"
#include "common/TimeSpanControl.hh"

#include "ui_watchrr_station.h"

#include <kvalobs/kvObsPgm.h>

#define MILOGGER_CATEGORY "kvhqc.StationDialog"
#include "util/HqcLogging.hh"

static const int MIN_DAYS = 7, MAX_DAYS = 120;

StationDialog::StationDialog(const Sensor& sensor, const TimeSpan& time, QWidget* parent)
    : QDialog(parent)
    , mSensor(sensor)
    , mObsPgmRequest(0)
{
  init();

  ui->editStation->setText(QString::number(mSensor.stationId));
  ui->dateFrom->setDateTime(timeutil::to_QDateTime(time.t0()));
  ui->dateTo  ->setDateTime(timeutil::to_QDateTime(time.t1()));
  ui->editStation->selectAll();
}

StationDialog::StationDialog(QWidget* parent)
    : QDialog(parent)
    , mSensor(0, kvalobs::PARAMID_RR_24, 0, 0, -1)
    , mObsPgmRequest(0)
{
  init();
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

  mTimeControl = new TimeSpanControl(this);
  mTimeControl->setMinimumGap(4*24);
  mTimeControl->install(ui->dateFrom, ui->dateTo);

  Helpers::installStationIdCompleter(this, ui->editStation);

  mTypesModel.reset(new TypeIdModel);
  ui->comboType->setModel(mTypesModel.get());

  ui->editStation->selectAll();
}

StationDialog::~StationDialog()
{
  METLIBS_LOG_SCOPE();
  delete mObsPgmRequest;
}

void StationDialog::onEditStation()
{
  METLIBS_LOG_SCOPE();
  checkStation();
  updateTypeList();
  enableOk();
}

void StationDialog::onEditTime()
{
  METLIBS_LOG_SCOPE();
  updateTypeList();
  enableOk();
}

void StationDialog::onSelectType(int)
{
  METLIBS_LOG_SCOPE();
  checkType();
  enableOk();
}

bool StationDialog::checkStation()
{
  METLIBS_LOG_SCOPE();
  METLIBS_LOG_DEBUG(LOGVAL(ui->editStation->text()));
  bool numberOk = false;
  const int stationId = ui->editStation->text().toInt(&numberOk);
  if (not numberOk) {
    ui->labelStationInfo->setText(tr("Cannot read station number"));
  } else if (not Helpers::isNorwegianStationId(stationId)) {
    ui->labelStationInfo->setText(tr("Invalid station number"));
  } else {
    mSensor.stationId = stationId;
    updateStationInfoText();

    delete mObsPgmRequest;
    mObsPgmRequest = new ObsPgmRequest(mSensor.stationId);
    connect(mObsPgmRequest, SIGNAL(complete()), this, SLOT(onObsPgmDone()));
    mObsPgmRequest->post();

    return true;
  } 
  mSensor.stationId = 0;
  return false;
}

void StationDialog::updateStationInfoText()
{
  QString name = "?";
  try {
    const kvalobs::kvStation& station = KvMetaDataBuffer::instance()->findStation(mSensor.stationId);
    name = QString::fromStdString(station.name());
  } catch (std::exception& e) {
    METLIBS_LOG_INFO("Station lookup problem, probably bad station number: " << e.what());
  }
  ui->labelStationInfo->setText(name);
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

void StationDialog::onObsPgmDone()
{
  METLIBS_LOG_SCOPE();
  updateTypeList();
}

void StationDialog::updateTypeList()
{
  METLIBS_LOG_SCOPE();
  std::set<int> typeIdSet;
  if (mSensor.stationId > 0 and mObsPgmRequest) {
    const hqc::hqcObsPgm_v& obs_pgm = mObsPgmRequest->get(mSensor.stationId);
    METLIBS_LOG_DEBUG(LOGVAL(obs_pgm.size()));
    if (obs_pgm.empty()) {
      ui->labelStationInfo->setText(tr("Unknown station (not in obs_pgm)"));
    } else {
      updateStationInfoText();
      const TimeSpan st = selectedTime();
      for (const kvalobs::kvObsPgm& op : obs_pgm) {
        if (st.intersection(TimeSpan(op.fromtime(), op.totime())).undef())
          continue;
        const int t = acceptThisObsPgm(op);
        if (t != 0)
          typeIdSet.insert(t);
      }
      if (typeIdSet.empty())
        ui->labelStationInfo->setText(tr("Could not find typeid"));
    }
  }
  const hqc::int_v newTypeIds(typeIdSet.begin(), typeIdSet.end());
  int newIdx = 0;
  if (newTypeIds != mTypesModel->values()) {
    const int currentIdx = ui->comboType->currentIndex();
    if (currentIdx >= 0) {
      const int currentTypeId = mTypesModel->values().at(currentIdx);
      hqc::int_v::const_iterator it = std::find(newTypeIds.begin(), newTypeIds.end(), currentTypeId);
      if (it != newTypeIds.end())
        newIdx = it - newTypeIds.end();
    }
    mTypesModel->setValues(newTypeIds);
    if (not newTypeIds.empty()) {
      ui->comboType->setCurrentIndex(newIdx);
      mSensor.typeId = mTypesModel->values().at(newIdx);
    } else {
      mSensor.typeId = 0;
    }
  }
}

bool StationDialog::valid() const
{
  return Helpers::isNorwegianStationId(mSensor.stationId)
      and mSensor.typeId != 0 and selectedTime().closed();
}

void StationDialog::enableOk()
{
  ui->buttonOK->setEnabled(valid());
}

TimeSpan StationDialog::selectedTime() const
{
  const timeutil::ptime tFrom = timeutil::from_QDateTime(QDateTime(ui->dateFrom->dateTime()));
  const timeutil::ptime tTo   = timeutil::from_QDateTime(QDateTime(ui->dateTo  ->dateTime()));
  return TimeSpan(tFrom, tTo);
}
