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

#ifndef DATACOLUMN_HH
#define DATACOLUMN_HH 1

#include "DataItem.hh"
#include "ObsColumn.hh"
#include "common/EditAccess.hh"
#include "common/TimeBuffer.hh"

class DataColumn : public ObsColumn
{ Q_OBJECT;

public:
  DataColumn(EditAccess_p ea, const Sensor& sensor, const TimeSpan& time, DataItem_p item);
  ~DataColumn();

  void setHeaderShowStation(bool show)
    { mHeaderShowStation = show; }

  void attach(ObsTableModel* table) override;
  void detach(ObsTableModel* table) override;

  Qt::ItemFlags flags(const timeutil::ptime& time) const override;
  QVariant data(const timeutil::ptime& time, int role) const override;
  bool setData(const timeutil::ptime& time, const QVariant& value, int role) override;
  QVariant headerData(Qt::Orientation orientation, int role) const override;

  virtual bool matchSensor(const Sensor& sensorObs) const;

  void setTimeOffset(const boost::posix_time::time_duration& timeOffset);

  const Sensor& sensor() const override;
  Time_s times() const override;
  int type() const override
    { return mItem->type(); }

  bool isBusy() const override
    { return mRequestBusy; }

  ObsData_pv getObs(const Time& time) const;

private:
  const boost::posix_time::time_duration& timeOffset() const
    { return mTimeOffset; }

  bool hasTimeOffset() const { return timeOffset().total_seconds() != 0; }

  SensorTime sensorTimeC2B(const Time& time) const
    { return SensorTime(sensor(), timeC2B(time)); }

  //! Convert buffer time to column time.
  /*! Subtracts timeOffset */
  Time timeB2C(const Time& time) const
    { return time - timeOffset(); }

  //! Convert column time to buffer time.
  /*! Adds timeOffset */
  Time timeC2B(const Time& time) const
    { return time + timeOffset(); }

  void makeBuffer();

private Q_SLOTS:
  void onBufferCompleted(const QString&);
  void onNewDataEnd(const ObsData_pv& data);
  void onUpdateDataEnd(const ObsData_pv& data);
  void onDropDataEnd(const SensorTime_v& dropped);

protected:
  EditAccess_p mDA;
  Sensor mSensor;
  TimeSpan mTimeSpan;
  TimeBuffer_p mBuffer;
  DataItem_p mItem;
  bool mAttached;
  bool mHeaderShowStation;
  boost::posix_time::time_duration mTimeOffset;
  bool mRequestBusy;
};

HQC_TYPEDEF_P(DataColumn);

#endif // DATACOLUMN_HH
