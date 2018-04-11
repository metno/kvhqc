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

#include "StationCardModel.hh"

#include "TasksColumn.hh"
#include "common/ColumnFactory.hh"
#include "common/ModelColumn.hh"
#include "common/KvHelpers.hh"
#include "util/Helpers.hh"

#define MILOGGER_CATEGORY "kvhqc.StationCardModel"
#include "common/ObsLogging.hh"

namespace /* anonymous */ {
const int timeOffsetsVx[] = {-18, -12, 0};
const int columnsVx[] = {kvalobs::PARAMID_V4, kvalobs::PARAMID_V5, kvalobs::PARAMID_V6};

const int columns[] = {kvalobs::PARAMID_RR_24, kvalobs::PARAMID_SA, kvalobs::PARAMID_SD, kvalobs::PARAMID_EE};
const ObsColumn::Type columnTypes[] = {ObsColumn::NEW_CORRECTED, ObsColumn::ORIGINAL, ObsColumn::NEW_CONTROLINFO};
} // namespace anonymous

StationCardModel::StationCardModel(TaskAccess_p da, ModelAccess_p ma, const Sensor& sensor, const TimeSpan& time)
    : WatchRRTableModel(da)
    , mRR24CorrectedColumn(-1)
    , mRR24OriginalColumn(-1)
{
  setTimeSpan(time);
  for (int timeOffsetVx : timeOffsetsVx) {
    for (int paramIdVx : columnsVx)
      addStationColumn(sensor, paramIdVx, ObsColumn::NEW_CORRECTED, timeOffsetVx);
  }

  for (ObsColumn::Type columnType : columnTypes) {
    for (int paramId : columns)
      addStationColumn(sensor, paramId, columnType, 0);
  }

  if (ma) {
    ModelColumn_p mc = ColumnFactory::columnForSensor(ma, sensor, time);
    mc->setHeaderShowStation(false);
    addColumn(mc);
  }
}

void StationCardModel::addStationColumn(const Sensor& sensor, int paramId, ObsColumn::Type columnType, int timeOffset)
{
  if (paramId == kvalobs::PARAMID_EE && sensor.typeId != 305)
    return;

  Sensor s(sensor);
  s.paramId = paramId;

  DataColumn_p dc = ColumnFactory::columnForSensor(mDA, s, mTime, columnType);
  if (not dc) {
    HQC_LOG_WARN("no column for sensor " << s);
    return;
  }

  dc->setHeaderShowStation(false);
  if (timeOffset != 0)
    dc->setTimeOffset(boost::posix_time::hours(timeOffset));

  if (paramId == kvalobs::PARAMID_RR_24 && columnType == ObsColumn::NEW_CORRECTED) {
    mRR24EditTime = std::make_shared<EditTimeColumn>(dc);
    addColumn(mRR24EditTime);
  } else {
    addColumn(std::make_shared<TasksColumn>(dc));
  }

  if (paramId == kvalobs::PARAMID_RR_24) {
    if (columnType == ObsColumn::NEW_CORRECTED)
      mRR24CorrectedColumn = countColumns() - 1;
    else if (columnType == ObsColumn::ORIGINAL)
      mRR24OriginalColumn = countColumns() - 1;
  }
}

int StationCardModel::getRR24CorrectedColumn() const
{
  return mRR24CorrectedColumn;
}

int StationCardModel::getRR24OriginalColumn() const
{
  return mRR24OriginalColumn;
}

void StationCardModel::setRR24TimeSpan(const TimeSpan& tr)
{
  mRR24EditTime->setEditableTime(tr);
}
