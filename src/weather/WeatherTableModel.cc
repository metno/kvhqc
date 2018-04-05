/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2013-2018 met.no

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

#include "WeatherTableModel.hh"

#include "common/KvMetaDataBuffer.hh"

#define MILOGGER_CATEGORY "kvhqc.WeatherTableModel"
#include "util/HqcLogging.hh"

const int WeatherTableModel::parameters[] = {
  211,214,216,213,215,262,178,173,177,1,61,81,86,87,83,90,15,14,55,104,108,
  109,110,112,18,7,273,41,31,32,33,42,43,34,36,38,40,
  23,24,22,403,404,131,134,151,154,250,221,9,12
};
const int WeatherTableModel::NPARAMETERS = sizeof(parameters)/sizeof(parameters[0]);

WeatherTableModel::WeatherTableModel(EditAccessPtr da, const Sensor& sensor, const TimeSpan& time, ObsColumn::Type t)
  : DataListModel(da, time)
{
  const KvMetaDataBuffer::ObsPgmList& opl = KvMetaDataBuffer::instance()->findObsPgm(sensor.stationId);
  for(int i=0; i<NPARAMETERS; ++i) {
    const int paramId = parameters[i];
    for (const kvalobs::kvObsPgm& op : opl) {
      if (paramId != op.paramID() or sensor.typeId != op.typeID())
        continue;
      if (time.intersection(TimeSpan(op.fromtime(), op.totime())).undef())
        continue;

      const Sensor s(sensor.stationId, paramId, sensor.level, sensor.sensor, sensor.typeId);
      DataColumnPtr oc = ColumnFactory::columnForSensor(da, s, time, t);
      if (oc) {
        oc->setHeaderShowStation(false);
        addColumn(oc);
      }
      break;
    }
  }
}
