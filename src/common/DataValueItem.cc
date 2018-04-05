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

#include "DataValueItem.hh"

#include "HqcUserConfig.hh"
#include "KvMetaDataBuffer.hh"
#include "ObsColumn.hh"
#include "ObsHelpers.hh"
#include "Tasks.hh"
#include "HqcApplication.hh"
#include "KvHelpers.hh"

#include <kvalobs/kvDataOperations.h>

#include <QApplication>
#include <QBrush>
#include <QFont>
#include <QVariant>

#define MILOGGER_CATEGORY "kvhqc.DataValueItem"
#include "util/HqcLogging.hh"

DataValueItem::DataValueItem(ObsColumn::Type columnType)
  : mColumnType(columnType)
{
}

ObsColumn::Type DataValueItem::type() const
{
  return mColumnType;
}

Qt::ItemFlags DataValueItem::flags(const ObsData_pv& obs) const
{
    Qt::ItemFlags f = DataItem::flags(obs);
    if (mColumnType == ObsColumn::NEW_CORRECTED)
        f |= Qt::ItemIsEditable;
    return f;
}

QVariant DataValueItem::data(const ObsData_pv& obs, const SensorTime& st, int role) const
{
  if (obs.empty())
    return QVariant();
  ObsData_p o = obs.front();

  const bool isNC = mColumnType == ObsColumn::NEW_CORRECTED;
  if (role == Qt::BackgroundRole) {
    if (isNC) {
#if 0
      if (obs->hasRequiredTasks())
        return QBrush(Qt::red);
      else if (obs->hasTasks())
        return QBrush(QColor(0xFF, 0x60, 0)); // red orange
#endif
    } else if (mColumnType == ObsColumn::ORIGINAL) {
      const int ui_2 = Helpers::extract_ui2(o);
      if (hqcApp) {
        const QColor bg = hqcApp->userConfig()->dataOrigUI2Background(ui_2);
        if (bg.isValid())
          return QBrush(bg);
      }
    }
  } else if (role == Qt::ForegroundRole) {
    // FIXME this is a hack, but the idea of having all non-numbers in dark gray is also mysterious
    const QVariant d = data(obs, st, Qt::DisplayRole);
    if (d.type() == QVariant::String) {
      bool ok = false;
      d.toFloat(&ok);
      if (!ok)
        return QColor(Qt::darkGray);
    }
    if (mColumnType != ObsColumn::ORIGINAL) {
      const kvalobs::kvControlInfo& ci = o->controlinfo();
      if (ci.flag(kvalobs::flag::fhqc) == 0) { // not hqc touched
        if (ci.qc2dDone())
          return QColor(Qt::darkMagenta);
        else if (ci.flag(kvalobs::flag::fnum) >= 6)
          return QColor(Qt::red);
      }
    }
  } else if (role == Qt::FontRole) {
    QFont f;
    if (o->isModified())
      f.setBold(true);
    return f;
  }
  return DataItem::data(obs, st, role);
}

float DataValueItem::getValue(ObsData_p obs) const
{
  if (!obs)
    return kvalobs::MISSING;
  else if (type() == ObsColumn::ORIGINAL)
    return obs->original();
  else
    return obs->corrected();
}
