
#include "TypeIdModel.hh"

#include "KvHelpers.hh"

#define MILOGGER_CATEGORY "kvhqc.TypeIdModel"
#include "util/HqcLogging.hh"

TypeIdModel::TypeIdModel(const std::vector<int>& typeIds)
  : mTypeIds(typeIds)
{
}

QVariant TypeIdModel::data(const QModelIndex& index, int role) const
{
  const int typeId = mTypeIds[index.row()];
  if (role == Qt::DisplayRole)
    return typeId;
  else if (role == Qt::ToolTipRole)
    return Helpers::typeInfo(typeId);
  else
    return QVariant();
}
