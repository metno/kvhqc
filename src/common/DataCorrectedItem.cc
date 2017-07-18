
#include "DataCorrectedItem.hh"

#include "ObsHelpers.hh"
#include "KvMetaDataBuffer.hh"
#include "ObsColumn.hh"
#include "Tasks.hh"
#include "util/stringutil.hh"
#include "common/KvHelpers.hh"

#include <QVariant>
#include <QApplication>
#include <QBrush>

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
