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

#include "DataItem.hh"
#include "ObsColumn.hh"

#include "util/make_set.hh"

#include <QVariant>

DataItem::~DataItem()
{
}

Sensor_s DataItem::sensors(const Sensor& base) const
{
  return make_set<Sensor_s>(base);
}

Qt::ItemFlags DataItem::flags(const ObsData_pv&) const
{
  return Qt::ItemIsEnabled|Qt::ItemIsSelectable;
}

QVariant DataItem::data(const ObsData_pv&, const SensorTime&, int role) const
{
  if (role == ObsColumn::ValueTypeRole)
    return ObsColumn::Numerical;
  else
    return QVariant();
}

bool DataItem::setData(const ObsData_pv&, EditAccess_p, const SensorTime&, const QVariant&, int)
{
  return false;
}

bool DataItem::matchSensor(const Sensor& sensorColumn, const Sensor& sensorObs) const
{
  const Sensor_s s = sensors(sensorColumn);
  return s.find(sensorObs) != s.end();
}
