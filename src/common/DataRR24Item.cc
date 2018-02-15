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


#include "DataRR24Item.hh"

#include "ObsHelpers.hh"

#include <QBrush>
#include <QVariant>

#define MILOGGER_CATEGORY "kvhqc.DataRR24Item"
#include "util/HqcLogging.hh"

DataRR24Item::DataRR24Item(Code2TextCPtr codes)
    : DataCorrectedItem(codes)
{
}

Qt::ItemFlags DataRR24Item::flags(ObsData_p obs) const
{
  Qt::ItemFlags f = DataCorrectedItem::flags(obs);
  if (obs) {
    const int typeId = obs->sensorTime().sensor.typeId;
    if (typeId == 302 or typeId == 305 or typeId == 402)
      f &= ~Qt::ItemIsEditable;
  }
  return f;
}

QVariant DataRR24Item::data(ObsData_p obs, const SensorTime& st, int role) const
{
  const QVariant d = DataCorrectedItem::data(obs, st, role);
  if (role == Qt::BackgroundRole and mColumnType == ObsColumn::NEW_CORRECTED
      and not d.isValid() and obs and Helpers::is_accumulation(obs))
  {
    return QBrush(Helpers::is_endpoint(obs) ? QColor(0xC0, 0xFF, 0xC0) : QColor(0xE0, 0xFF, 0xE0));
  }
  return d;
}
