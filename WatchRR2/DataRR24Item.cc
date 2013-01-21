
#include "DataRR24Item.hh"

#include "Helpers.hh"

#include <QtCore/QVariant>
#include <QtGui/QBrush>

#define NDEBUG
#include "debug.hh"

DataRR24Item::DataRR24Item(bool showNew, Code2TextPtr codes)
    : DataCorrectedItem(showNew, codes)
{
}

Qt::ItemFlags DataRR24Item::flags() const
{
    return DataItem::flags() & ~Qt::ItemIsEditable;
}

QVariant DataRR24Item::data(EditDataPtr obs, int role) const
{
    const QVariant d = DataCorrectedItem::data(obs, role);
    if (role == Qt::BackgroundRole and mShowNew and not d.isValid() and obs and Helpers::is_accumulation(obs))
        return QBrush(Helpers::is_endpoint(obs) ? QColor(0xC0, 0xFF, 0xC0) : QColor(0xE0, 0xFF, 0xE0));
    return d;
}

bool DataRR24Item::setData(EditDataPtr, EditAccessPtr, const SensorTime&, const QVariant&, int)
{
    throw "cannot set RR24 value";
    return false;
}