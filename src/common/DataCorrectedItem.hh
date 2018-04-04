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


#ifndef DATACORRECTEDITEM_HH
#define DATACORRECTEDITEM_HH 1

#include "DataCodeItem.hh"

class DataCorrectedItem : public DataCodeItem {
public:
  DataCorrectedItem(Code2TextCPtr codes);
  
  QVariant data(const ObsData_pv& obs, const SensorTime& st, int role) const override;
  bool setData(const ObsData_pv&, EditAccess_p da, const SensorTime& st, const QVariant& value, int role) override;
  QString description(bool mini) const override;
};

HQC_TYPEDEF_P(DataCorrectedItem);

#endif // DATACORRECTEDITEM_HH
