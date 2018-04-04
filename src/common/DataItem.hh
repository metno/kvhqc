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

#ifndef DATAITEM_HH
#define DATAITEM_HH 1

#include "common/EditAccess.hh"
#include "ObsColumn.hh"

class DataItem {
public:
  virtual ~DataItem();

  virtual Qt::ItemFlags flags(ObsData_p obs) const;
  virtual QVariant data(ObsData_p obs, const SensorTime& st, int role) const;
  virtual bool setData(ObsData_p obs, EditAccess_p ea, const SensorTime& st, const QVariant& value, int role);
  virtual QString description(bool mini) const = 0;
  virtual bool matchSensor(const Sensor& sensorColumn, const Sensor& sensorObs) const;
  virtual ObsColumn::Type type() const = 0;
};

HQC_TYPEDEF_P(DataItem);

#endif // DATAITEM_HH
