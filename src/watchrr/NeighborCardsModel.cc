/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2018 met.no

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

#include "NeighborCardsModel.hh"

#include "common/ColumnFactory.hh"
#include "common/KvHelpers.hh"
#include "common/BufferHelpers.hh"
#include "common/NeighborHeader.hh"
#include "common/ObsPgmRequest.hh"
#include "common/SensorHeader.hh"
#include "util/Helpers.hh"
#include "util/make_set.hh"

#define MILOGGER_CATEGORY "kvhqc.NeighborCardsModel"
#include "common/ObsLogging.hh"

namespace /* anonymous */ {
const int N_COLUMNS = 18;
const int columnPars[N_COLUMNS] = {
  kvalobs::PARAMID_V4, kvalobs::PARAMID_V5, kvalobs::PARAMID_V6,
  kvalobs::PARAMID_V4, kvalobs::PARAMID_V5, kvalobs::PARAMID_V6,
  kvalobs::PARAMID_V4, kvalobs::PARAMID_V5, kvalobs::PARAMID_V6,
  kvalobs::PARAMID_RR_24, kvalobs::PARAMID_SA, kvalobs::PARAMID_SD,
  kvalobs::PARAMID_RR_24, kvalobs::PARAMID_SA, kvalobs::PARAMID_SD,
  kvalobs::PARAMID_RR_24, kvalobs::PARAMID_SA, kvalobs::PARAMID_SD
};
const int N_UNIQUE_PARS = 6;
const int uniqueColumnPars[N_UNIQUE_PARS] = {
  kvalobs::PARAMID_V4, kvalobs::PARAMID_V5, kvalobs::PARAMID_V6,
  kvalobs::PARAMID_RR_24, kvalobs::PARAMID_SA, kvalobs::PARAMID_SD
};
const ObsColumn::Type columnTypes[N_COLUMNS] = {
  ObsColumn::NEW_CORRECTED,   ObsColumn::NEW_CORRECTED,   ObsColumn::NEW_CORRECTED,
  ObsColumn::NEW_CORRECTED,   ObsColumn::NEW_CORRECTED,   ObsColumn::NEW_CORRECTED,
  ObsColumn::NEW_CORRECTED,   ObsColumn::NEW_CORRECTED,   ObsColumn::NEW_CORRECTED,
  ObsColumn::NEW_CORRECTED,   ObsColumn::NEW_CORRECTED,   ObsColumn::NEW_CORRECTED,
  ObsColumn::ORIGINAL,        ObsColumn::ORIGINAL,        ObsColumn::ORIGINAL,
  ObsColumn::NEW_CONTROLINFO, ObsColumn::NEW_CONTROLINFO, ObsColumn::NEW_CONTROLINFO
};
const int columnTimeOffsets[N_COLUMNS] = {
  -18, -18, -18,
  -12, -12, -12,
  0, 0, 0,
  0, 0, 0,
  0, 0, 0,
  0, 0, 0
};
} // namespace anonymous

NeighborCardsModel::NeighborCardsModel(TaskAccess_p da/*, ModelAccessPtr ma*/, const Sensor& sensor, const TimeSpan& timeRange)
  : mDA(da)
  , mTimeSpan(timeRange)
  , mTime(mTimeSpan.t0())
  , mSensors(1, sensor)
{
  METLIBS_LOG_SCOPE(LOGVAL(sensor));
  hqc::int_s stationIds = KvMetaDataBuffer::instance()->findNeighborStationIds(sensor.stationId);
  stationIds.insert(sensor.stationId);

  ObsPgmRequest* mObsPgmRequest = new ObsPgmRequest(stationIds);
  mObsPgmRequest->sync();
  
  KvMetaDataBuffer::instance()->addNeighbors(mSensors, sensor, mTimeSpan, mObsPgmRequest, 20);
  mItems.reserve(N_COLUMNS);
  mTimeOffsets.reserve(N_COLUMNS);
  METLIBS_LOG_DEBUG(LOGVAL(mSensors.size()));

  Sensor_s sensors;
  for(int i=0; i<N_COLUMNS; ++i) {
    const Sensor s(sensor.stationId, columnPars[i], sensor.level, sensor.sensor, sensor.typeId);
    DataItem_p item = ColumnFactory::itemForSensor(da, s, columnTypes[i]);
    mItems.push_back(item);
    for(const Sensor& base : mSensors) {
      const Sensor bs(base.stationId, columnPars[i], base.level, base.sensor, base.typeId);
      METLIBS_LOG_DEBUG(LOGVAL(bs) << LOGVAL(sensors.size()));
      insert_all(sensors, item->sensors(bs));
    }

    mTimeOffsets.push_back(boost::posix_time::hours(columnTimeOffsets[i]));
  }
  delete mObsPgmRequest;

  METLIBS_LOG_DEBUG(LOGVAL(sensors.size()));
  mBuffer = std::make_shared<TimeBuffer>(sensors, mTimeSpan);
  mBuffer->syncRequest(mDA);
}

NeighborCardsModel::~NeighborCardsModel()
{
}

int NeighborCardsModel::rowCount(const QModelIndex&) const
{
  return mSensors.size();
}

int NeighborCardsModel::columnCount(const QModelIndex&) const
{
  return mItems.size();
}

Qt::ItemFlags NeighborCardsModel::flags(const QModelIndex& index) const
{
  return getItem(index)->flags(getObs(index)) & ~Qt::ItemIsEditable;
}

QVariant NeighborCardsModel::data(const QModelIndex& index, int role) const
{
  return getItem(index)->data(getObs(index), getSensorTime(index), role);
}

void NeighborCardsModel::setTime(const timeutil::ptime& time)
{
  if (not mTimeSpan.contains(time) or mTime == time)
    return;

  mTime = time;
  Q_EMIT dataChanged(createIndex(0,0), createIndex(mSensors.size()-1, mItems.size()-1));
  Q_EMIT timeChanged(mTime);
}

QVariant NeighborCardsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole or role == Qt::ToolTipRole) {
    if (orientation == Qt::Horizontal) {
      const Sensor s(-1, columnPars[section], -1, -1, -1);
      SensorHeader sh(s, SensorHeader::NEVER, SensorHeader::ALWAYS, columnTimeOffsets[section]);
      return sh.sensorHeader(mItems[section], orientation, role);
    } else if (role == Qt::ToolTipRole) {
      SensorHeader sh(mSensors[section], SensorHeader::ALWAYS, SensorHeader::NEVER, 0);
      return sh.sensorHeader(DataItem_p(), orientation, role);
    } else {
      return NeighborHeader::headerData(mSensors[0].stationId, mSensors[section].stationId, orientation, role);
    }
  }
  return QVariant();
}

ObsData_pv NeighborCardsModel::getObs(const QModelIndex& index) const
{
  return Helpers::getObs(mBuffer, getItem(index), getSensorTime(index));
}

SensorTime NeighborCardsModel::getSensorTime(const QModelIndex& index) const
{
  Sensor sensor = mSensors[index.row()];
  sensor.paramId = columnPars[index.column()];
  return SensorTime(sensor, getTime(index));
}

std::vector<int> NeighborCardsModel::neighborStations() const
{
  std::vector<int> n;
  n.reserve(mSensors.size());
  for (const Sensor& s : mSensors)
    n.push_back(s.stationId);
  return n;
}
