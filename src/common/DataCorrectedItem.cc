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


#include "DataCorrectedItem.hh"

#include "KvMetaDataBuffer.hh"
#include "ObsColumn.hh"
#include "ObsHelpers.hh"
#include "Tasks.hh"
#include "common/KvHelpers.hh"
#include "util/stringutil.hh"

#include <QApplication>
#include <QBrush>
#include <QVariant>

#define MILOGGER_CATEGORY "kvhqc.DataCorrectedItem"
#include "common/ObsLogging.hh"

DataCorrectedItem::DataCorrectedItem(Code2TextCPtr codes)
  : DataCodeItem(ObsColumn::NEW_CORRECTED, codes)
{
}

QVariant DataCorrectedItem::data(ObsData_p obs, const SensorTime& st, int role) const
{
  if (obs and (role == Qt::ToolTipRole or role == Qt::StatusTipRole)) {
    QString tip;
#if 0
    if (mColumnType == ObsColumn::NEW_CORRECTED)
      tip = tasks::asText(obs->allTasks());
#endif
    return Helpers::appendedText(tip, DataCodeItem::data(obs, st, role).toString());
  }
  return DataCodeItem::data(obs, st, role);
}

bool DataCorrectedItem::setData(ObsData_p obs, EditAccess_p da, const SensorTime& st, const QVariant& value, int role)
{
  if (role != Qt::EditRole or mColumnType != ObsColumn::NEW_CORRECTED)
    return false;

  const QString svalue = value.toString();
  try {
    const float newC = mCodes->fromText(svalue);
    const bool reject = (newC == kvalobs::REJECTED);
    if (reject and not obs)
      return false;
    if (KvMetaDataBuffer::instance()->checkPhysicalLimits(st, newC) == CachedParamLimits::OutsideMinMax)
      return false;
    
    da->newVersion();
    ObsUpdate_p update;
    if (not obs)
      update = da->createUpdate(st);
    else
      update = da->createUpdate(obs);
    
    if (reject)
      Helpers::reject(update, obs);
    else
      Helpers::auto_correct(update, obs, newC);
    da->storeUpdates(ObsUpdate_pv(1, update));
    return true;
  } catch (std::exception& e) {
    METLIBS_LOG_INFO("exception while editing data for sensor/time " << st
        << " svalue='" << svalue << "': " << e.what());
    return false;
  }
}

QString DataCorrectedItem::description(bool mini) const
{
  if (mini)
    return qApp->translate("DataColumn", "corr");
  else
    return qApp->translate("DataColumn", "corrected");
}
