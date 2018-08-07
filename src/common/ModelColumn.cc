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


#include "ModelColumn.hh"

#include "ModelAccess.hh"
#include "SensorHeader.hh"

#define MILOGGER_CATEGORY "kvhqc.ModelColumn"
#include "common/ObsLogging.hh"

ModelColumn::ModelColumn(ModelAccess_p ma, const Sensor& sensor, const TimeSpan&)
    : mBuffer(new ModelBuffer(ma))
    , mSensor(sensor)
    , mHeaderShowStation(true)
    , mCodes(std::make_shared<Code2Text>())
{
  mSensor.typeId = 0;
}

ModelColumn::~ModelColumn()
{
}

Qt::ItemFlags ModelColumn::flags(const timeutil::ptime& /*time*/) const
{
  return Qt::ItemIsEnabled|Qt::ItemIsSelectable;
}

QVariant ModelColumn::data(const timeutil::ptime& time, int role) const
{
  ModelData_p mdl = get(time);
  if (not mdl)
    return QVariant();

  const float value = mdl->value();
  if (role == Qt::ToolTipRole or role == Qt::StatusTipRole) {
    return mCodes->asTip(value);
  } else if (role == Qt::DisplayRole or role == Qt::EditRole) {
    return mCodes->asText(value);
  } else if (role == Qt::TextAlignmentRole) {
    return Qt::AlignRight;
  }
  return QVariant();
}

bool ModelColumn::setData(const timeutil::ptime& /*time*/, const QVariant& /*value*/, int /*role*/)
{
  return false;
}

QVariant ModelColumn::headerData(Qt::Orientation orientation, int role) const
{
  SensorHeader sh(mSensor, mHeaderShowStation ? SensorHeader::ALWAYS : SensorHeader::TOOLTIP,
      SensorHeader::ALWAYS, mTimeOffset.hours());
  return sh.modelHeader(orientation, role);
}

void ModelColumn::setTimeOffset(const boost::posix_time::time_duration& timeOffset)
{
  mTimeOffset = timeOffset;
}

void ModelColumn::setCodes(Code2TextCPtr codes)
{
  mCodes = codes;
}

void ModelColumn::bufferReceived(const ModelData_pv& mdata)
{
  Time_s times;
  for (ModelData_p mdl : mdata)
    times.insert(mdl->sensorTime().time - mTimeOffset);
  Q_EMIT columnChanged(times, shared_from_this());
}

ModelData_p ModelColumn::get(const timeutil::ptime& time) const
{
  return mBuffer->get(SensorTime(mSensor, time + mTimeOffset));
}
