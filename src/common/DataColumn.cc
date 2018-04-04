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


#include "DataColumn.hh"

#include "BufferHelpers.hh"
#include "SensorHeader.hh"
#include "util/make_set.hh"

#define MILOGGER_CATEGORY "kvhqc.DataColumn"
#include "common/ObsLogging.hh"

DataColumn::DataColumn(EditAccess_p da, const Sensor& sensor, const TimeSpan& t, DataItem_p item)
    : mDA(da)
    , mSensor(sensor)
    , mTimeSpan(t)
    , mItem(item)
    , mAttached(false)
    , mHeaderShowStation(true)
    , mRequestBusy(false)
{
  METLIBS_LOG_SCOPE(LOGVAL(sensor) << LOGVAL(t));
  makeBuffer();
}

void DataColumn::makeBuffer()
{
  mBuffer = std::make_shared<TimeBuffer>(mItem->sensors(sensor()), mTimeSpan.shifted(timeOffset())); // add time offset, column -> buffer
  TimeBuffer* b = mBuffer.get();
  connect(b, &TimeBuffer::bufferCompleted, this, &DataColumn::onBufferCompleted);
  connect(b, &TimeBuffer::newDataEnd, this, &DataColumn::onNewDataEnd);
  connect(b, &TimeBuffer::updateDataEnd, this, &DataColumn::onUpdateDataEnd);
  connect(b, &TimeBuffer::dropDataEnd, this, &DataColumn::onDropDataEnd);
}
      
DataColumn::~DataColumn()
{
}

void DataColumn::attach(ObsTableModel* otm)
{
  METLIBS_LOG_SCOPE();
  ObsColumn::attach(otm);
  mAttached = true;

  mRequestBusy = true;
  Q_EMIT columnBusyStatus(mRequestBusy);
  mBuffer->postRequest(mDA);
}

void DataColumn::detach(ObsTableModel* otm)
{
  METLIBS_LOG_SCOPE();
  mRequestBusy = false;
  Q_EMIT columnBusyStatus(mRequestBusy);
  mBuffer = TimeBuffer_p();
  mAttached = false;

  ObsColumn::detach(otm);
}

Qt::ItemFlags DataColumn::flags(const timeutil::ptime& time) const
{
  return mItem->flags(getObs(time));
}
      
QVariant DataColumn::data(const timeutil::ptime& time, int role) const
{
  const SensorTime st = sensorTimeC2B(time);
  return mItem->data(getObs(time), st, role);
}

bool DataColumn::setData(const timeutil::ptime& time, const QVariant& value, int role)
{
  METLIBS_LOG_SCOPE();
  try {
    METLIBS_LOG_DEBUG(LOGVAL(value.toString()));
    return mItem->setData(getObs(time), mDA, sensorTimeC2B(time), value, role);
  } catch (std::exception& e) {
    HQC_LOG_WARN(e.what());
    return false;
  }
}

QVariant DataColumn::headerData(Qt::Orientation orientation, int role) const
{
  SensorHeader sh(sensor(), mHeaderShowStation ? SensorHeader::ALWAYS : SensorHeader::TOOLTIP,
      SensorHeader::ALWAYS, mTimeOffset.hours());
  return sh.sensorHeader(mItem, orientation, role);
}
      
void DataColumn::onBufferCompleted(const QString&)
{
  METLIBS_LOG_SCOPE(LOGVAL(sensor()));
  Q_EMIT columnTimesChanged(shared_from_this());
  mRequestBusy = false;
  Q_EMIT columnBusyStatus(mRequestBusy);
}

void DataColumn::onNewDataEnd(const ObsData_pv& data)
{
  METLIBS_LOG_SCOPE();
  if (not mRequestBusy)
    onUpdateDataEnd(data);
}

void DataColumn::onUpdateDataEnd(const ObsData_pv& data)
{
  METLIBS_LOG_SCOPE();
  for (ObsData_p obs : data)
    Q_EMIT columnChanged(timeB2C(obs->sensorTime().time), shared_from_this());
}

void DataColumn::onDropDataEnd(const SensorTime_v& dropped)
{
  METLIBS_LOG_SCOPE();
  for (const SensorTime& st : dropped)
    Q_EMIT columnChanged(timeB2C(st.time), shared_from_this());
}

Time_s DataColumn::times() const
{
  if (!hasTimeOffset())
    return mBuffer->times();

  Time_s offsetTimes;
  for (const Time& t : mBuffer->times())
    offsetTimes.insert(timeB2C(t));
  return offsetTimes;
}

ObsData_pv DataColumn::getObs(const Time& time) const
{
  return Helpers::getObs(mBuffer, mItem->sensors(sensor()), timeC2B(time));
}

bool DataColumn::matchSensor(const Sensor& sensorObs) const
{
  return mItem->matchSensor(sensor(), sensorObs);
}

const Sensor& DataColumn::sensor() const
{
  return mSensor;
}

void DataColumn::setTimeOffset(const boost::posix_time::time_duration& timeOffset)
{
  if (timeOffset != mTimeOffset) {
    mTimeOffset = timeOffset;
    makeBuffer();
  }
}
