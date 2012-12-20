
#include "RR24Column.hh"

#include "Helpers.hh"
#include <QtGui/QBrush>

#define NDEBUG
#include "debug.hh"

RR24Column::RR24Column(EditAccessPtr da, const Sensor& sensor, DisplayType displayType)
    : DataColumn(da, sensor, displayType)
{
    setEditable(false);
}

QVariant RR24Column::data(const timeutil::ptime& time, int role) const
{
    using namespace Helpers;

    if( role == Qt::BackgroundRole and mDisplayType == NEW_CORRECTED ) {
        EditDataPtr obs = getObs(time);
        if( obs and (not obs->hasTasks()) and is_accumulation(obs) )
            return QBrush(is_endpoint(obs) ? QColor(0xC0, 0xFF, 0xC0) : QColor(0xE0, 0xFF, 0xE0));
    }
    return DataColumn::data(time, role);
}
