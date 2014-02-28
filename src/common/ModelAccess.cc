
#include "ModelAccess.hh"

ModelAccess::~ModelAccess()
{
}

bool ModelAccess::lt_ModelSensor::operator()(const Sensor& a, const Sensor& b) const
{
  if (a.stationId != b.stationId)
    return a.stationId < b.stationId;
  if (a.paramId != b.paramId)
    return a.paramId < b.paramId;
  return a.level < b.level;
}

bool ModelAccess::eq_ModelSensor::operator()(const Sensor& a, const Sensor& b) const
{
  return (a.stationId == b.stationId)
      and (a.paramId == b.paramId)
      and (a.level == b.level);
}

bool ModelAccess::lt_ModelSensorTime::operator()(const SensorTime& a, const SensorTime& b) const
{
  if (lt_ModelSensor()(a.sensor, b.sensor))
    return true;
  if (not eq_ModelSensor()(a.sensor, b.sensor))
    return false;
  return a.time < b.time;
}
