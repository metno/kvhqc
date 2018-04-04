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

#ifndef DATAVALUEITEM_HH
#define DATAVALUEITEM_HH 1

#include "DataItem.hh"

class DataValueItem : public DataItem {
public:
  DataValueItem(ObsColumn::Type columnType);

  Qt::ItemFlags flags(ObsData_p obs) const override;
  QVariant data(ObsData_p obs, const SensorTime& st, int role) const override;
  ObsColumn::Type type() const override;

protected:
  virtual float getValue(ObsData_p obs) const;

protected:
  ObsColumn::Type mColumnType;
};

HQC_TYPEDEF_P(DataValueItem);

#endif // DATAVALUEITEM_HH
