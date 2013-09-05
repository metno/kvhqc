
#include "DataCodeItem.hh"

#define MILOGGER_CATEGORY "kvhqc.DataCodeItem"
#include "HqcLogging.hh"

DataCodeItem::DataCodeItem(ObsColumn::Type columnType, Code2TextCPtr codes)
  : DataValueItem(columnType)
  , mCodes(codes)
{
}

QVariant DataCodeItem::data(EditDataPtr obs, const SensorTime& st, int role) const
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

  if (role == Qt::ForegroundRole) {
    if (mCodes->isCode(getValue(obs)))
      return Qt::darkGray;
  } else if (role == Qt::DisplayRole or role == Qt::EditRole) {
    return mCodes->asText(getValue(obs), role == Qt::EditRole);
  } else if (role == Qt::TextAlignmentRole) {
    return Qt::AlignVCenter+(mCodes->isCode(getValue(obs)) ? Qt::AlignLeft : Qt::AlignRight);
  }
  return DataValueItem::data(obs, st, role);
}
