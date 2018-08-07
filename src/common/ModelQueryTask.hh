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


#ifndef COMMON_MODELQUERYTASK_HH
#define COMMON_MODELQUERYTASK_HH 1

#include "ModelData.hh"
#include "QueryTask.hh"

class ModelQueryTask : public QueryTask
{ Q_OBJECT;
public:
  ModelQueryTask(const SensorTime_v& sensorTimes, size_t priority);
  ~ModelQueryTask();

  QString querySql(QString dbversion) const override;
  void notifyRow(const ResultRow& row) override;
  void notifyDone(const QString& withError) override;

Q_SIGNALS:
  void data(const ModelData_pv&);

private:
  void sendData();

private:
  SensorTime_v mSensorTimes;
  ModelData_pv mData;
};

#endif // COMMON_MODELQUERYTASK_HH
