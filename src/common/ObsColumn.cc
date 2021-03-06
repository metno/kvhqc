
#include "ObsColumn.hh"

Qt::ItemFlags ObsColumn::flags(const timeutil::ptime& /*time*/) const
{
    return Qt::ItemIsEnabled;
}

bool ObsColumn::setData(const timeutil::ptime& /*time*/, const QVariant& /*value*/, int /*role*/)
{
    return false;
}

Sensor ObsColumn::sensor() const
{
    return Sensor(); // invalid sensor
}
