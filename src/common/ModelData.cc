
#include "ModelData.hh"

bool lt_ModelSensor::operator()(const Sensor& a, const Sensor& b) const
{
  if (a.stationId != b.stationId)
    return a.stationId < b.stationId;
  if (a.paramId != b.paramId)
    return a.paramId < b.paramId;
  return a.level < b.level;
}

bool eq_ModelSensor::operator()(const Sensor& a, const Sensor& b) const
{
  return (a.stationId == b.stationId)
      and (a.paramId == b.paramId)
      and (a.level == b.level);
}

bool lt_ModelSensorTime::operator()(const SensorTime& a, const SensorTime& b) const
{
  if (lt_ModelSensor()(a.sensor, b.sensor))
    return true;
  if (not eq_ModelSensor()(a.sensor, b.sensor))
    return false;
  return a.time < b.time;
}

bool eq_ModelSensorTime::operator()(const SensorTime& a, const SensorTime& b) const
{
  return eq_ModelSensor()(a.sensor, b.sensor)
      and a.time == b.time;
}
