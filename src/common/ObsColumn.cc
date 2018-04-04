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

#include "ObsColumn.hh"

void ObsColumn::attach(ObsTableModel*)
{
}

void ObsColumn::detach(ObsTableModel*)
{
}

Qt::ItemFlags ObsColumn::flags(const timeutil::ptime& /*time*/) const
{
    return Qt::ItemIsEnabled;
}

bool ObsColumn::setData(const timeutil::ptime& /*time*/, const QVariant& /*value*/, int /*role*/)
{
    return false;
}

const Sensor& ObsColumn::sensor() const
{
  static const Sensor invalid;
  return invalid;
}

Time_s ObsColumn::times() const
{
  return Time_s();
}
