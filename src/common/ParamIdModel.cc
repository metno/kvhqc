
#include "ParamIdModel.hh"

#include "KvHelpers.hh"

#define MILOGGER_CATEGORY "kvhqc.ParamIdModel"
#include "util/HqcLogging.hh"

ParamIdModel::ParamIdModel(const std::vector<int>& paramIds)
    : mParamIds(paramIds)
{
}

QVariant ParamIdModel::data(const QModelIndex& index, int role) const
{
  const int paramId = mParamIds[index.row()];
  if (role == Qt::DisplayRole) {
    return Helpers::parameterName(paramId);
  } else if (role == Qt::ToolTipRole) {
    return Helpers::paramInfo(paramId);
  } else {
    return QVariant();
  }
}
