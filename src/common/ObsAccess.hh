
#ifndef ObsAccess_hh
#define ObsAccess_hh 1

#include "ObsSubscription.hh"
#include "ObsUpdate.hh"
#include <boost/signals.hpp>
#include <set>
#include <vector>

class ObsAccess : public boost::enable_shared_from_this<ObsAccess>, private boost::noncopyable {
public:
  virtual ~ObsAccess();
  
  typedef std::set<timeutil::ptime> TimeSet;
  virtual TimeSet allTimes(const std::vector<Sensor>& sensors, const TimeRange& limits) = 0;
  TimeSet allTimes(const Sensor& sensor, const TimeRange& limits)
    { return allTimes(std::vector<Sensor>(1, sensor), limits); }

  void addAllTimes(TimeSet& times, const std::vector<Sensor>& sensors, const TimeRange& limits);
  void addAllTimes(TimeSet& times, const Sensor& sensor, const TimeRange& limits)
    { return addAllTimes(times, std::vector<Sensor>(1, sensor), limits); }
  
  struct lt_ObsDataPtr  : public std::binary_function<ObsDataPtr, ObsDataPtr, bool> {
    bool operator()(const ObsDataPtr& a, const ObsDataPtr& b) const
      { return lt_SensorTime()(a->sensorTime(), b->sensorTime()); }
  };
  typedef std::set<ObsDataPtr, lt_ObsDataPtr> DataSet;
  virtual DataSet allData(const std::vector<Sensor>& sensors, const TimeRange& limits) = 0;
  DataSet allData(const Sensor& sensor, const TimeRange& limits)
    { return allData(std::vector<Sensor>(1, sensor), limits); }

  virtual ObsDataPtr find(const SensorTime& st) = 0;
  virtual ObsDataPtr create(const SensorTime& st) = 0;
  virtual bool update(const std::vector<ObsUpdate>& updates) = 0;

  virtual void addSubscription(const ObsSubscription& s) = 0;
  virtual void removeSubscription(const ObsSubscription& s) = 0;

public:
  enum ObsDataChange { MODIFIED, CREATED, DESTROYED };
  boost::signal2<void, ObsDataChange, ObsDataPtr> obsDataChanged;
};
typedef boost::shared_ptr<ObsAccess> ObsAccessPtr;

#endif // ObsAccess_hh
