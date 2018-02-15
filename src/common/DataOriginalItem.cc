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


#include "DataOriginalItem.hh"

#include "ObsHelpers.hh"
#include "util/stringutil.hh"

#include <QApplication>
#include <QBrush>
#include <QVariant>

#define MILOGGER_CATEGORY "kvhqc.DataOriginalItem"
#include "util/HqcLogging.hh"

DataOriginalItem::DataOriginalItem(Code2TextCPtr codes)
  : DataCodeItem(ObsColumn::ORIGINAL, codes)
{
}

QVariant DataOriginalItem::data(ObsData_p obs, const SensorTime& st, int role) const
{
  if (not obs)
    return QVariant();
  
  if (role == Qt::ToolTipRole or role == Qt::StatusTipRole) {
    QString tip;
    const int ui_2 = Helpers::extract_ui2(obs);
    if (ui_2 == 3)
      tip = qApp->translate("DataOriginalItem", "surely wrong");
    else if (ui_2 == 2)
      tip = qApp->translate("DataOriginalItem", "very suspicious (probably wrong)");
    else if (ui_2 == 1)
      tip = qApp->translate("DataOriginalItem", "suspicious (probably ok)");
    else if (ui_2 == 9)
      tip = qApp->translate("DataOriginalItem", "no quality info available");
    return Helpers::appendedText(tip, DataCodeItem::data(obs, st, role).toString());
  }
  return DataCodeItem::data(obs, st, role);
}

QString DataOriginalItem::description(bool mini) const
{
  if (mini)
    return qApp->translate("DataColumn", "orig");
  else
    return qApp->translate("DataColumn", "original");
}
