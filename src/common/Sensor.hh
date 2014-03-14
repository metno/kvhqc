
#ifndef COMMON_SENSOR_HH
#define COMMON_SENSOR_HH 1

#include "util/boostutil.hh"
#include "util/timeutil.hh"
#include <functional>
#include <set>
#include <vector>

/*! Identifies a sensor. This is a measurement device at a specific
 *  station with a specific communication channel.
 */
struct Sensor {
  int stationId, paramId, level, sensor, typeId;
  Sensor(int s, int p, int l, int sensor, int t)
    : stationId(s), paramId(p), level(l), sensor(sensor), typeId(t) { }
  Sensor()
    : stationId(0), paramId(0), level(0), sensor(0), typeId(0) { }
  bool valid() const
    { return stationId>0 && paramId>0 && level>=0 && sensor>=0 && typeId!=0; }
};

//! Ordering for \c Sensor objects.
struct lt_Sensor : public std::binary_function<Sensor, Sensor, bool> {
  bool operator()(const Sensor& a, const Sensor& b) const;
};

//! Equality of \c Sensor objects.
struct eq_Sensor : public std::binary_function<Sensor, Sensor, bool> {
  bool operator()(const Sensor& a, const Sensor& b) const;
};

typedef std::set<Sensor, lt_Sensor> Sensor_s;

// ========================================================================

/*! Identifies an observation by \c Sensor and time. */
struct SensorTime {
  Sensor sensor;
  timeutil::ptime time;
  SensorTime(const Sensor& s, const timeutil::ptime& t)
    : sensor(s), time(t) { }
  SensorTime() { }
  bool valid() const
    { return sensor.valid() and not time.is_not_a_date_time(); }
};

//! Ordering for \c SensorTime objects.
struct lt_SensorTime : public std::binary_function<SensorTime, SensorTime, bool> {
  bool operator()(const SensorTime& a, const SensorTime& b) const;
};

//! Equality of \c SensorTime objects.
struct eq_SensorTime : public std::binary_function<SensorTime, SensorTime, bool> {
  bool operator()(const SensorTime& a, const SensorTime& b) const;
};

HQC_TYPEDEF_V(SensorTime);

#endif // COMMON_SENSOR_HH
