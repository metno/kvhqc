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


#ifndef WATCHRR_NEIGHBORCARDSMODEL_HH
#define WATCHRR_NEIGHBORCARDSMODEL_HH 1

#include "TaskAccess.hh"
#include "common/TimeBuffer.hh"
#include "common/DataItem.hh"
#include <vector>

#include <QAbstractTableModel>

class NeighborCardsModel : public QAbstractTableModel
{   Q_OBJECT;
public:
  NeighborCardsModel(TaskAccess_p da/*, ModelAccessPtr ma*/, const Sensor& sensor, const TimeSpan& timeRange);
  virtual ~NeighborCardsModel();

  virtual int rowCount(const QModelIndex&) const;
  virtual int columnCount(const QModelIndex&) const;
  virtual Qt::ItemFlags flags(const QModelIndex& index) const;
  virtual QVariant data(const QModelIndex& index, int role) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

  void setTime(const timeutil::ptime& time);
  const timeutil::ptime& getTime() const
    { return mTime; }

  std::vector<int> neighborStations() const;

Q_SIGNALS:
  void timeChanged(const timeutil::ptime& time);

private:
  ObsData_pv getObs(const QModelIndex& index) const;
  SensorTime getSensorTime(const QModelIndex& index) const;
  DataItem_p getItem(const QModelIndex& index) const
    { return mItems[index.column()]; }
  timeutil::ptime getTime(const QModelIndex& index) const
    { return mTime + mTimeOffsets[index.column()]; }

private:
  TaskAccess_p mDA;
  TimeBuffer_p mBuffer;
  TimeSpan mTimeSpan;
  timeutil::ptime mTime;

  // rows
  std::vector<DataItem_p> mItems;
  std::vector<boost::posix_time::time_duration> mTimeOffsets;

  // columns
  std::vector<Sensor> mSensors;
};

#endif /* WATCHRR_NEIGHBORCARDSMODEL_HH */
