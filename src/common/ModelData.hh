
#ifndef MODELDATA_HH
#define MODELDATA_HH 1

#include "Sensor.hh"

#include "util/boostutil.hh"

class ModelData {
public:
  ModelData(const SensorTime& st, float value)
    : mSensorTime(st), mValue(value) { }

  virtual ~ModelData() { }

  const SensorTime& sensorTime() const
    { return mSensorTime; }

  virtual float value() const
    { return mValue; }

private:
  SensorTime mSensorTime;
  float mValue;
};

HQC_TYPEDEF_P(ModelData);
HQC_TYPEDEF_PV(ModelData);

//! Ordering for \c SensorTime objects regarding only data relevat for models (ie. no sensor or typeId).
struct lt_ModelSensorTime : public std::binary_function<SensorTime, SensorTime, bool> {
  bool operator()(const SensorTime& a, const SensorTime& b) const;
};

struct eq_ModelSensorTime : public std::binary_function<SensorTime, SensorTime, bool> {
  bool operator()(const SensorTime& a, const SensorTime& b) const;
};

struct eq_ModelSensor : public std::binary_function<Sensor, Sensor, bool> {
  bool operator()(const Sensor& a, const Sensor& b) const;
};

struct lt_ModelSensor : public std::binary_function<Sensor, Sensor, bool> {
  bool operator()(const Sensor& a, const Sensor& b) const;
};

struct lt_ModelData_p  : public std::binary_function<ModelData_p, ModelData_p, bool> {
  bool operator()(const ModelData_p& a, const ModelData_p& b) const
    { return lt_ModelSensorTime()(a->sensorTime(), b->sensorTime()); }
  bool operator()(const SensorTime& a, const ModelData_p& b) const
    { return lt_ModelSensorTime()(a, b->sensorTime()); }
  bool operator()(const ModelData_p& a, const SensorTime& b) const
    { return lt_ModelSensorTime()(a->sensorTime(), b); }
};
typedef std::set<ModelData_p, lt_ModelData_p> ModelData_ps;

#endif // MODELDATA_HH
