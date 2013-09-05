
#include "ParamIdModel.hh"

#include "KvMetaDataBuffer.hh"

ParamIdModel::ParamIdModel(const std::vector<int>& paramIds)
    : mParamIds(paramIds)
{
}

QVariant ParamIdModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::DisplayRole)
        return QString::fromStdString(KvMetaDataBuffer::instance()->findParamName(mParamIds[index.row()]));
    else
        return QVariant();
}

