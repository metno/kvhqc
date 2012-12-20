
#include "Sensor.hh"

bool lt_Sensor::operator()(const Sensor& a, const Sensor& b) const
{
    if (a.stationId != b.stationId)
        return a.stationId < b.stationId;
    if (a.paramId != b.paramId)
        return a.paramId < b.paramId;
    if (a.level != b.level)
        return a.level < b.level;
    if (a.sensor != b.sensor)
        return a.sensor < b.sensor;
    return a.typeId < b.typeId;
}

bool eq_Sensor::operator()(const Sensor& a, const Sensor& b) const
{
    return (a.stationId == b.stationId)
        and (a.paramId == b.paramId)
        and (a.level == b.level)
        and (a.sensor == b.sensor)
        and (a.typeId == b.typeId);
}

bool lt_SensorTime::operator()(const SensorTime& a, const SensorTime& b) const
{
    if (lt_Sensor()(a.sensor, b.sensor))
        return true;
    if (not eq_Sensor()(a.sensor, b.sensor))
        return false;
    return a.time < b.time;
}

bool eq_SensorTime::operator()(const SensorTime& a, const SensorTime& b) const
{
    return eq_Sensor()(a.sensor, b.sensor) and a.time == b.time;
}
