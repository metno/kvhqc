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


#include "DataControlinfoItem.hh"

#include "KvHelpers.hh"

#include <QApplication>
#include <QFont>
#include <QVariant>

DataControlinfoItem::DataControlinfoItem()
{
}

QVariant DataControlinfoItem::data(const ObsData_pv& obs, const SensorTime& st, int role) const
{
  if (obs.size() == 1) {
    ObsData_p o = obs.front();

    if (role == Qt::ToolTipRole or role == Qt::StatusTipRole) {
      return Helpers::getFlagExplanation(getControlinfo(o));
    } else if (role == Qt::DisplayRole or role == Qt::EditRole) {
      return Helpers::getFlagText(getControlinfo(o));
#if 0
    } else if (role == Qt::FontRole) {
      QFont f;
      if (obs->modifiedControlinfo())
        f.setBold(true);
      return f;
#endif
    }
  }
  return DataItem::data(obs, st, role);
}

QString DataControlinfoItem::description(bool mini) const
{
  if (mini)
    return qApp->translate("DataColumn", "flags");
  else
    return qApp->translate("DataColumn", "controlflags");
}

ObsColumn::Type DataControlinfoItem::type() const
{
  return ObsColumn::NEW_CONTROLINFO;
}

const kvalobs::kvControlInfo& DataControlinfoItem::getControlinfo(ObsData_p obs) const
{
  if (!obs)
    throw std::runtime_error("no obs");
  return obs->controlinfo();
}
