
#include "DataItem.hh"
#include "ObsColumn.hh"

#include <QtCore/QStringList>
#include <QtCore/QVariant>

DataItem::~DataItem()
{
}

Qt::ItemFlags DataItem::flags() const
{
    return Qt::ItemIsEnabled|Qt::ItemIsSelectable;
}

QVariant DataItem::data(EditDataPtr, int role) const
{
    if (role == ObsColumn::ValueTypeRole)
        return ObsColumn::Numerical;
    else if (role == ObsColumn::TextCodesRole)
        return QStringList();
    else
        return QVariant();
}


bool DataItem::setData(EditDataPtr, EditAccessPtr, const SensorTime&, const QVariant&, int)
{
    throw std::runtime_error("cannot set original value");
    return false;
}
