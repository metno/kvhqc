
#ifndef ModelAccess_hh
#define ModelAccess_hh 1

#include "ModelData.hh"
#include "Sensor.hh"
#include <boost/signals.hpp>
#include <set>
#include <vector>

class ModelAccess : public boost::enable_shared_from_this<ModelAccess>, private boost::noncopyable {
public:
  enum { MODEL_TYPEID=99, MODEL_SENSOR=0 };

  virtual ~ModelAccess();
  virtual ModelDataPtr find(const SensorTime& st) = 0;

  struct lt_ModelDataPtr  : public std::binary_function<ModelDataPtr, ModelDataPtr, bool> {
    bool operator()(const ModelDataPtr& a, const ModelDataPtr& b) const
      { return lt_SensorTime()(a->sensorTime(), b->sensorTime()); }
  };
  typedef std::set<ModelDataPtr, lt_ModelDataPtr> ModelDataSet;
  virtual ModelDataSet findMany(const std::vector<SensorTime>& sensorTimes) = 0;

  static bool isModelSensor(const Sensor& s)
    { return s.sensor == MODEL_SENSOR and s.typeId == MODEL_TYPEID; }
  static bool isModelSensorTime(const SensorTime& st)
    { return isModelSensor(st.sensor); }

  static Sensor makeModelSensor(const Sensor& s)
    { Sensor ms(s); ms.sensor = MODEL_SENSOR; ms.typeId = MODEL_TYPEID; return ms; }
  static SensorTime makeModelSensorTime(const SensorTime& st)
    { return SensorTime(makeModelSensor(st.sensor), st.time); }

public:
  boost::signal1<void, ModelDataPtr> modelDataChanged;
};
typedef boost::shared_ptr<ModelAccess> ModelAccessPtr;

#endif // ModelAccess_hh
