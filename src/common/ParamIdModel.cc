
#include "ParamIdModel.hh"

#include "KvMetaDataBuffer.hh"

#define MILOGGER_CATEGORY "kvhqc.ParamIdModel"
#include "util/HqcLogging.hh"

ParamIdModel::ParamIdModel(const std::vector<int>& paramIds)
    : mParamIds(paramIds)
{
}

QVariant ParamIdModel::data(const QModelIndex& index, int role) const
{
  const int paramId = mParamIds[index.row()];
  try {
    const kvalobs::kvParam& p = KvMetaDataBuffer::instance()->findParam(paramId);
    if (role == Qt::DisplayRole)
      return QString::fromStdString(p.name());
    if (role == Qt::ToolTipRole)
      return QString::number(paramId) + ": " + QString::fromStdString(p.description());
  } catch (std::exception& e) {
    HQC_LOG_WARN("paramId " << paramId << " not known");
  }
  if (role == Qt::DisplayRole)
    return QString("{%1}").arg(paramId);
  return QVariant();
}
