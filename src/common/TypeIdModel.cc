
#include "TypeIdModel.hh"

#include "KvHelpers.hh"

#define MILOGGER_CATEGORY "kvhqc.TypeIdModel"
#include "util/HqcLogging.hh"

TypeIdModel::TypeIdModel(const std::vector<int>& typeIds)
  : mTypeIds(typeIds)
{
}

void TypeIdModel::addTypeOverride(int typeId, const QString& format, const QString& label)
{
  mTypeOverrides[typeId] = TypeOverrideData_t(label.isEmpty() ? format : label, format);
}

QVariant TypeIdModel::data(const QModelIndex& index, int role) const
{
  if (role != Qt::DisplayRole and role != Qt::ToolTipRole)
    return QVariant();

  const int typeId = mTypeIds[index.row()];
  TypeOverrides_t::const_iterator it = mTypeOverrides.find(typeId);
  if (it != mTypeOverrides.end()) {
    if (role == Qt::DisplayRole)
      return it->second.first;
    else
      return it->second.second;
  } else {
    if (role == Qt::DisplayRole)
      return typeId;
    else
      return Helpers::typeInfo(typeId);
  }
}
