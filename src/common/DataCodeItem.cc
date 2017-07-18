
#include "DataCodeItem.hh"

#define MILOGGER_CATEGORY "kvhqc.DataCodeItem"
#include "util/HqcLogging.hh"

#include <QColor>

DataCodeItem::DataCodeItem(ObsColumn::Type columnType, Code2TextCPtr codes)
  : DataValueItem(columnType)
  , mCodes(codes)
{
}

QVariant DataCodeItem::data(ObsData_p obs, const SensorTime& st, int role) const
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
    
    if (not obs)
      return QVariant();
    
    if (role == Qt::ForegroundRole) {
      if (mCodes->isCode(getValue(obs)))
        return QColor(Qt::darkGray);
    } else if (role == Qt::DisplayRole) {
      return mCodes->asText(getValue(obs), false);
    } else if (role == Qt::EditRole) {
      return "";
    } else if (role == Qt::TextAlignmentRole) {
      return Qt::AlignVCenter+(mCodes->isCode(getValue(obs)) ? Qt::AlignLeft : Qt::AlignRight);
    }
  }
  return DataValueItem::data(obs, st, role);
}
