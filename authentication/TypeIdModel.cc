
#include "TypeIdModel.hh"

#include "KvMetaDataBuffer.hh"

#define MILOGGER_CATEGORY "kvhqc.TypeIdModel"
#include "HqcLogging.hh"

TypeIdModel::TypeIdModel(const std::vector<int>& typeIds)
  : mTypeIds(typeIds)
{
}

QVariant TypeIdModel::data(const QModelIndex& index, int role) const
{
  const int typeId = mTypeIds[index.row()];
  if (role == Qt::DisplayRole)
    return typeId;
  if (role == Qt::ToolTipRole) {
    try {
      const kvalobs::kvTypes& t = KvMetaDataBuffer::instance()->findType(typeId);
      return QString::fromStdString(t.format());
    } catch (std::exception& e) {
      HQC_LOG_WARN("typeId " << typeId << " not known, exception is: " << e.what());
    }
  }
  return QVariant();
}
