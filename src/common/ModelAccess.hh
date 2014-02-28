
#ifndef ModelAccess_hh
#define ModelAccess_hh 1

#include "ModelData.hh"
#include "Sensor.hh"
#include "TimeRange.hh"

#include <boost/signals.hpp>

#include <set>
#include <vector>

class ModelAccess : public boost::enable_shared_from_this<ModelAccess>, private boost::noncopyable {
public:
  enum { MODEL_TYPEID=99, MODEL_SENSOR=0 };

  virtual ~ModelAccess();
  virtual ModelDataPtr find(const SensorTime& st) = 0;

  //! Ordering for \c SensorTime objects regarding only data relevat for models (ie. no sensor or typeId).
  struct lt_ModelSensorTime : public std::binary_function<SensorTime, SensorTime, bool> {
    bool operator()(const SensorTime& a, const SensorTime& b) const;
  };

  struct eq_ModelSensor : public std::binary_function<Sensor, Sensor, bool> {
    bool operator()(const Sensor& a, const Sensor& b) const;
  };

  struct lt_ModelSensor : public std::binary_function<Sensor, Sensor, bool> {
    bool operator()(const Sensor& a, const Sensor& b) const;
  };

  struct lt_ModelDataPtr  : public std::binary_function<ModelDataPtr, ModelDataPtr, bool> {
    bool operator()(const ModelDataPtr& a, const ModelDataPtr& b) const
      { return lt_ModelSensorTime()(a->sensorTime(), b->sensorTime()); }
  };
  typedef std::set<ModelDataPtr, lt_ModelDataPtr> ModelDataSet;
  virtual ModelDataSet findMany(const std::vector<SensorTime>& sensorTimes) = 0;
  virtual ModelDataSet allData(const std::vector<Sensor>& sensors, const TimeRange& limits) = 0;

public:
  boost::signal1<void, ModelDataPtr> modelDataChanged;
};
typedef boost::shared_ptr<ModelAccess> ModelAccessPtr;

#endif // ModelAccess_hh
