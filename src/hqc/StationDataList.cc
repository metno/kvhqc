/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2014-2018 met.no

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

#include "StationDataList.hh"

#include "common/ColumnFactory.hh"
#include "common/DataListModel.hh"
#include "common/HqcSystemDB.hh"
#include "common/KvHelpers.hh"
#include "common/ObsPgmRequest.hh"

#include <QCheckBox>
#include <QComboBox>

#include "ui_datalist.h"

#define MILOGGER_CATEGORY "kvhqc.StationDataList"
#include "common/ObsLogging.hh"

namespace {
const std::string VIEW_TYPE = "StationDataList";
}

StationDataList::StationDataList(QWidget* parent)
    : ObsPgmDataList(parent)
{
  setWindowTitle(tr("Station Data"));
  setWindowIcon(QIcon("icons:weatherstation.svg"));

  mCheckAggregated = new QCheckBox(tr("Aggregated"), this);
  mCheckAggregated->setToolTip(tr("Show aggregated parameters"));
  mCheckAggregated->setChecked(true);
  mCheckAllTypeIds = new QCheckBox(tr("All Types"), this);
  mCheckAllTypeIds->setToolTip(tr("Show all typeids"));
  mCheckAllTypeIds->setChecked(false);
  mComboParamGroups = new QComboBox(this);
  mComboParamGroups->setToolTip(tr("On: show all parameters\nOff: hide some parameters that are normally not checked manually"));
  mComboParamGroups->addItem(tr("All Parameters"));
  for (const auto& pg : HqcSystemDB::paramGroups())
    mComboParamGroups->addItem(pg.label);

  connect(mCheckAggregated, &QCheckBox::toggled, this, &StationDataList::updateModel);
  connect(mCheckAllTypeIds, &QCheckBox::toggled, this, &StationDataList::updateModel);
  connect(mComboParamGroups, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &StationDataList::onComboParamGroupsChanged);

  ui->layoutFilters->addWidget(mCheckAggregated);
  ui->layoutFilters->addWidget(mCheckAllTypeIds);
  ui->layoutFilters->addWidget(mComboParamGroups);
}

StationDataList::~StationDataList()
{
}

SensorTime StationDataList::sensorSwitch() const
{
  const SensorTime& sst = storeSensorTime(), cst = currentSensorTime();
  if (sst.sensor.stationId == cst.sensor.stationId
      and std::abs(sst.sensor.typeId) == std::abs(cst.sensor.typeId)
      and timeSpan().contains(cst.time))
    return sst;

  Sensor s = cst.sensor;
  if (s.typeId < 0)
    s.typeId = -s.typeId;
  s.paramId = 1;
  s.level = 0;
  s.sensor = 0;
  return SensorTime(s, cst.time);
}

void StationDataList::addSensorColumn(const Sensor& s, ObsColumn::Type type)
{
  if (DataColumn_p oc = ColumnFactory::columnForSensor(mDA, s, timeSpan(), type))
    model()->addColumn(oc);
}

void StationDataList::addSensorColumns(Sensor_s& alreadyShown, const Sensor& add)
{
  if (alreadyShown.insert(add).second) {
    addSensorColumn(add, ObsColumn::ORIGINAL);
    addSensorColumn(add, ObsColumn::NEW_CORRECTED);
  }

  if (!mCheckAggregated->isChecked())
    return;
  
  hqc::int_s aggregatedTo;
  Helpers::aggregatedParameters(add.paramId, aggregatedTo);
  Sensor agg(add);
  agg.typeId = -add.typeId;
  for (int a : aggregatedTo) {
    agg.paramId = a;
    if (alreadyShown.insert(agg).second)
      addSensorColumn(agg, ObsColumn::NEW_CORRECTED);
  }
}

void StationDataList::updateModel()
{
  METLIBS_LOG_SCOPE(LOGVAL(currentSensorTime()) << LOGVAL(timeSpan()));

  Sensor s(currentSensorTime().sensor);
  if (s.typeId < 0)
    s.typeId = -s.typeId;
  const TimeSpan& time = timeSpan();

  Sensor_s columnSensors; // to avoid duplicate columns

  model()->removeAllColumns();
  model()->setTimeSpan(time);
  if (!mObsPgmRequest)
    return;

  const bool includeAllTypeIds = mCheckAllTypeIds->isChecked();
  const hqc::hqcObsPgm_v& opl = mObsPgmRequest->get(s.stationId);
  hqc::int_v paramIds;
  const int DUMMY_PARAMID_ALL = 0;
  if (mComboParamGroups->currentIndex() != 0) {
    const QString group = mComboParamGroups->currentText();
    for (const auto& pg : HqcSystemDB::paramGroups()) {
      if (pg.label == group) {
        paramIds.reserve(pg.paramIds.size());
        for (const auto& p : pg.paramIds) {
            paramIds.push_back(p.paramId);
        }
      }
    }
  } else {
    paramIds.push_back(DUMMY_PARAMID_ALL);
  }
  for (int paramId : paramIds) {
    for (const hqc::hqcObsPgm& op : opl) {
      if (paramId != DUMMY_PARAMID_ALL && paramId != op.paramID())
        continue;
      if (!includeAllTypeIds && s.typeId != op.typeID())
        continue;
      if (time.intersection(TimeSpan(op.fromtime(), op.totime())).undef())
        continue;

      addSensorColumns(columnSensors, Sensor(s.stationId, op.paramID(), s.level, s.sensor, op.typeID()));
    }
  }
}

void StationDataList::onComboParamGroupsChanged(int)
{
  updateModel();
  reselectCurrent();
}

std::string StationDataList::viewType() const
{
  return VIEW_TYPE;
}
