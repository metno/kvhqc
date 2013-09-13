
#define MILOGGER_CATEGORY "kvhqc.ObsLogging" // only to get rid of warning
#include "ObsLogging.hh"

#include "Sensor.hh"

std::ostream& operator<<(std::ostream& out, const Sensor& s)
{
  out << "(s:" << s.stationId
      << ", p:" << s.paramId
      << ", l:" << s.level
      << ", s:" << s.sensor
      << ", t:" << s.typeId << ')';
  return out;
}

std::ostream& operator<<(std::ostream& out, const SensorTime& st)
{
  out << st.sensor << '@' << st.time;
  return out;
}
