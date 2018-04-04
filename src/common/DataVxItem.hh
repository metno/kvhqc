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

#ifndef DATAVXITEM_HH
#define DATAVXITEM_HH 1

#include "DataValueItem.hh"

class DataVxItem : public DataValueItem {
public:
  DataVxItem(ObsColumn::Type columnType, EditAccess_p da);

  Sensor_s sensors(const Sensor& base) const override;

  QVariant data(const ObsData_pv& obs, const SensorTime& st, int role) const override;
  bool setData(const ObsData_pv& obs, EditAccess_p da, const SensorTime& st, const QVariant& value, int role) override;
  QString description(bool mini) const override;

private:
  typedef std::pair<int,int> Codes_t;
  Codes_t getCodes(ObsData_p obs1, ObsData_p obs2) const;

private:
  EditAccess_p mDA;
};

#endif // DATAVXITEM_HH
