
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
  : DataValueItem(showNew ? ObsColumn::NEW_CORRECTED : ObsColumn::OLD_CORRECTED)
  , mCodes(codes)
{
}

QVariant DataCorrectedItem::data(EditDataPtr obs, int role) const
{
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

  if (not obs)
    return QVariant();
  
  if (role == Qt::ToolTipRole or role == Qt::StatusTipRole) {
    QString tip = mCodes->asTip(getValue(obs));
    return Helpers::appendText(tip, DataValueItem::data(obs, role).toString());
  } else if (role == Qt::DisplayRole or role == Qt::EditRole) {
    return mCodes->asText(getValue(obs));
  } else if (role == Qt::TextAlignmentRole) {
    return Qt::AlignVCenter+(mCodes->isCode(getValue(obs)) ? Qt::AlignLeft : Qt::AlignRight);
  }
  return DataValueItem::data(obs, role);
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
    METLIBS_LOG_ERROR("exception while editing data for sensor/time " << st << ": " << e.what());
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
