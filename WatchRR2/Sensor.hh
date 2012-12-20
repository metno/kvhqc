
#ifndef SENSOR_HH
#define SENSOR_HH 1

#include "timeutil.hh"
#include <functional>

struct Sensor {
    int stationId, paramId, level, sensor, typeId;
    Sensor(int s, int p, int l, int sensor, int t)
        : stationId(s), paramId(p), level(l), sensor(sensor), typeId(t) { }
};

struct lt_Sensor : public std::binary_function<Sensor, Sensor, bool> {
    bool operator()(const Sensor& a, const Sensor& b) const;
};

struct eq_Sensor : public std::binary_function<Sensor, Sensor, bool> {
    bool operator()(const Sensor& a, const Sensor& b) const;
};

// ========================================================================

struct SensorTime {
    Sensor sensor;
    timeutil::ptime time;
    SensorTime(const Sensor& s, const timeutil::ptime& t)
        : sensor(s), time(t) { }
};

struct lt_SensorTime : public std::binary_function<SensorTime, SensorTime, bool> {
    bool operator()(const SensorTime& a, const SensorTime& b) const;
};

struct eq_SensorTime : public std::binary_function<SensorTime, SensorTime, bool> {
    bool operator()(const SensorTime& a, const SensorTime& b) const;
};

#endif // SENSOR_HH
