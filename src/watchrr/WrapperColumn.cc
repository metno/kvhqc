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

#include "WrapperColumn.hh"

#include "common/DataColumn.hh"

#define MILOGGER_CATEGORY "kvhqc.WrapperColumn"
#include "util/HqcLogging.hh"

WrapperColumn::WrapperColumn(DataColumn_p dc)
  : mDC(dc)
{
  connect(dc.get(), &DataColumn::columnChanged, this, &WrapperColumn::columnChanged);
  connect(dc.get(), &DataColumn::columnTimesChanged, this, &WrapperColumn::columnTimesChanged);
}

WrapperColumn::~WrapperColumn()
{
}

void WrapperColumn::attach(ObsTableModel* table)
{
  mDC->attach(table);
}

void WrapperColumn::detach(ObsTableModel* table)
{
  mDC->detach(table);
}

Qt::ItemFlags WrapperColumn::flags(const timeutil::ptime& time) const
{
  return mDC->flags(time);
}

QVariant WrapperColumn::data(const timeutil::ptime& time, int role) const
{
  return mDC->data(time, role);
}

bool WrapperColumn::setData(const timeutil::ptime& time, const QVariant& value, int role)
{
  return mDC->setData(time, value, role);
}

QVariant WrapperColumn::headerData(Qt::Orientation orientation, int role) const
{
  return mDC->headerData(orientation, role);
}

int WrapperColumn::type() const
{
  return mDC->type();
}
