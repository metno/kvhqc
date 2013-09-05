
#include "DataCorrectedItem.hh"

#include "Helpers.hh"
#include "KvMetaDataBuffer.hh"
#include "ObsColumn.hh"
#include "Tasks.hh"

#include <QtCore/QVariant>
#include <QtGui/QApplication>
#include <QtGui/QBrush>

#define MILOGGER_CATEGORY "kvhqc.DataCorrectedItem"
#include "HqcLogging.hh"

DataCorrectedItem::DataCorrectedItem(bool showNew, Code2TextCPtr codes)
  : DataCodeItem(showNew ? ObsColumn::NEW_CORRECTED : ObsColumn::OLD_CORRECTED, codes)
{
}

QVariant DataCorrectedItem::data(EditDataPtr obs, const SensorTime& st, int role) const
{
  if (obs and (role == Qt::ToolTipRole or role == Qt::StatusTipRole)) {
    QString tip;
    if (mColumnType == ObsColumn::NEW_CORRECTED)
      tip = tasks::asText(obs->allTasks());
    return Helpers::appendedText(tip, DataCodeItem::data(obs, st, role).toString());
  }
  return DataCodeItem::data(obs, st, role);
}

bool DataCorrectedItem::setData(EditDataPtr obs, EditAccessPtr da, const SensorTime& st, const QVariant& value, int role)
{
  if (role != Qt::EditRole or mColumnType != ObsColumn::NEW_CORRECTED)
    return false;

  try {
    const float newC = mCodes->fromText(value.toString());
    const bool reject = (newC == kvalobs::REJECTED);
    if (reject and not obs)
      return false;
    if (not KvMetaDataBuffer::instance()->checkPhysicalLimits(st.sensor.paramId, newC))
      return false;
    
    da->newVersion();
    if (not obs)
      obs = da->createE(st);
    
    if (reject)
      Helpers::reject(da->editor(obs));
    else
      Helpers::auto_correct(da->editor(obs), newC);
    return true;
  } catch (std::exception& e) {
    HQC_LOG_ERROR("exception while editing data for sensor/time " << st << ": " << e.what());
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
