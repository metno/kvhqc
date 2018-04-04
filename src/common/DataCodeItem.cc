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

#include "DataCodeItem.hh"

#define MILOGGER_CATEGORY "kvhqc.DataCodeItem"
#include "util/HqcLogging.hh"

#include <QColor>

DataCodeItem::DataCodeItem(ObsColumn::Type columnType, Code2TextCPtr codes)
  : DataValueItem(columnType)
  , mCodes(codes)
{
}

QVariant DataCodeItem::data(const ObsData_pv& obs, const SensorTime& st, int role) const
{
  if (mCodes) {
    if (role == ObsColumn::ValueTypeRole or role == ObsColumn::TextCodesRole) {
      const QStringList allCodes = mCodes->allCodes();
      if (role == ObsColumn::TextCodesRole)
        return allCodes;
      int valueTypes = ObsColumn::Numerical;
      if (not allCodes.empty())
        valueTypes |= ObsColumn::TextCode;
      return valueTypes;
    } else if (role ==  ObsColumn::TextCodeExplanationsRole) {
      return mCodes->allExplanations();
    }

    if (obs.size() != 1)
      return QVariant();
    
    if (role == Qt::ForegroundRole) {
      if (mCodes->isCode(getValue(obs.front())))
        return QColor(Qt::darkGray);
    } else if (role == Qt::DisplayRole) {
      return mCodes->asText(getValue(obs.front()), false);
    } else if (role == Qt::EditRole) {
      return "";
    } else if (role == Qt::TextAlignmentRole) {
      return Qt::AlignVCenter + (mCodes->isCode(getValue(obs.front())) ? Qt::AlignLeft : Qt::AlignRight);
    }
  }
  return DataValueItem::data(obs, st, role);
}
