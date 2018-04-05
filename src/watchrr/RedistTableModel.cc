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

#include "RedistTableModel.hh"

#include "AnalyseRR24.hh"
#include "common/ColumnFactory.hh"
#include "common/Functors.hh"
#include "common/KvHelpers.hh"
#include "common/KvMetaDataBuffer.hh"

#include <kvalobs/kvDataOperations.h>

#define MILOGGER_CATEGORY "kvhqc.RedistTableModel"
#include "util/HqcLogging.hh"

static const int COLUMN_NEW = 2;

RedistTableModel::RedistTableModel(TaskAccess_p da, const Sensor& sensor, const TimeSpan& time)
    : WatchRRTableModel(da)
    , mSensor(sensor)
    , mRR24Codes(ColumnFactory::codesForParam(kvalobs::PARAMID_RR_24))
{
  setTimeSpan(time);
  addColumn(ColumnFactory::columnForSensor(mDA, mSensor, time, ObsColumn::ORIGINAL));
  addColumn(ColumnFactory::columnForSensor(mDA, mSensor, time, ObsColumn::NEW_CORRECTED));
  addColumn(ObsColumn_p()); // must be COLUMN_NEW

  const int nDays = mTime.days() + 1;
  for(int d=0; d<nDays; ++d) {
    ObsData_p obs = ta()->findE(SensorTime(mSensor, timeAtRow(d)));
    if (not obs)
      mNewValues.push_back(kvalobs::MISSING);
    else
      mNewValues.push_back(obs->corrected());
  }
}

RedistTableModel::~RedistTableModel()
{
  METLIBS_LOG_SCOPE();
}

Qt::ItemFlags RedistTableModel::flags(const QModelIndex& index) const
{
  if (not getColumn(index.column()))
    return Qt::ItemIsEnabled|Qt::ItemIsEditable;
  return (ObsTableModel::flags(index) & ~(Qt::ItemIsSelectable|Qt::ItemIsEditable));
}

QVariant RedistTableModel::data(const QModelIndex& index, int role) const
{
  if ((role == Qt::DisplayRole or role == Qt::EditRole) and not getColumn(index.column())) {
    return mRR24Codes->asText(mNewValues.at(index.row()));
  } else if ((role == Qt::ToolTipRole or role == Qt::StatusTipRole) and not getColumn(index.column())) {
    return mRR24Codes->asTip(mNewValues.at(index.row()));
  } else {
    return ObsTableModel::data(index, role);
  }
}

QVariant RedistTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole and orientation == Qt::Horizontal and not getColumn(section))
    return "RR_24\nnew";
  return ObsTableModel::headerData(section, orientation, role);
}

bool RedistTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if (role != Qt::EditRole or getColumn(index.column()))
    return false;

  try {
    const float rrNew = mRR24Codes->fromText(value.toString());
    if (KvMetaDataBuffer::instance()->checkPhysicalLimits(SensorTime(mSensor, timeAtRow(index.row())), rrNew) == CachedParamLimits::OutsideMinMax)
      return false;

    const int row = index.row();
    const float rrOld = mNewValues.at(row);
    if (fabs(rrNew - rrOld) < 0.05)
      return false;

    mNewValues.at(row) = rrNew;
    dataChanged(index, index);
    return true;
  } catch (std::exception&) {
    return false;
  }
}

float RedistTableModel::originalSum() const
{
  return RR24::calculateOriginalSum(ta(), mSensor, mTime);
}

float RedistTableModel::currentSum() const
{
  float sum = 0;
  for (float v : mNewValues)
    if (v >= 0)
      sum += v;
  return sum;
}

bool RedistTableModel::hasManualChanges() const
{
  const int nDays = mTime.days() + 1;
  for (int d=0; d<nDays; ++d) {
    ObsData_p obs = ta()->findE(SensorTime(mSensor, timeAtRow(d)));
    if (not obs and mNewValues.at(d) != kvalobs::MISSING)
      return true;
    else if (obs and not Helpers::float_eq()(mNewValues.at(d), obs->corrected()))
      return true;
  }
  return false;
}

void RedistTableModel::setNewCorrected(const std::vector<float>& newCorrected)
{
  METLIBS_LOG_SCOPE(LOGVAL(newCorrected.size()));
  if (newCorrected.size() != mNewValues.size())
    return;

  mNewValues = newCorrected;
  Q_EMIT dataChanged(index(0, COLUMN_NEW), index(mNewValues.size()-1, COLUMN_NEW));
}
