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


#ifndef WRAPPERCOLUMN_HH
#define WRAPPERCOLUMN_HH 1

#include "common/ObsColumn.hh"

class DataColumn;
HQC_TYPEDEF_P(DataColumn);

class WrapperColumn : public ObsColumn {
protected:
  WrapperColumn(DataColumn_p dc);

public:
  ~WrapperColumn();
  
  void attach(ObsTableModel* table) override;

  void detach(ObsTableModel* table) override;

  Qt::ItemFlags flags(const timeutil::ptime& time) const override;

  QVariant data(const timeutil::ptime& time, int role) const override;

  bool setData(const timeutil::ptime& time, const QVariant& value, int role) override;

  QVariant headerData(Qt::Orientation orientation, int role) const override;
  
  int type() const override;

protected:
  DataColumn_p mDC;
};

#endif // WRAPPERCOLUMN_HH
