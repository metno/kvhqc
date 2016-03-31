
#ifndef COMMON_OBSACCESS_HH
#define COMMON_OBSACCESS_HH 1

#include "ObsUpdate.hh"
#include "TimeRange.hh"
#include <boost/signals.hpp>
#include <set>
#include <vector>

/*! Access to observations.
 * This interface allows retrieving, creating, and storing observations.
 */
class ObsAccess : public boost::enable_shared_from_this<ObsAccess>, private boost::noncopyable {
public:
  virtual ~ObsAccess();

  typedef std::set<timeutil::ptime> TimeSet;

  /*! Make a list of observation times for which observations exist for the given sensors and time limits.
   * \param sensors the sensors to look at
   * \param limits the time limits
   * \param a set of times for which observations exist for at least one of the sensors
   */
  virtual TimeSet allTimes(const std::vector<Sensor>& sensors, const TimeRange& limits) = 0;

  /*! Same as allTimes, but for a single sensor.
   */
  TimeSet allTimes(const Sensor& sensor, const TimeRange& limits)
    { return allTimes(std::vector<Sensor>(1, sensor), limits); }

  /*! Adds result of allTimes to the already given set of times.
   * \param times existing set of times; this will possibly be extended
   */
  void addAllTimes(TimeSet& times, const std::vector<Sensor>& sensors, const TimeRange& limits);

  /*! Same as addAllTimes, but for a single sensor.
   */
  void addAllTimes(TimeSet& times, const Sensor& sensor, const TimeRange& limits)
    { return addAllTimes(times, std::vector<Sensor>(1, sensor), limits); }

  struct lt_ObsDataPtr  : public std::binary_function<ObsDataPtr, ObsDataPtr, bool> {
    bool operator()(const ObsDataPtr& a, const ObsDataPtr& b) const
      { return lt_SensorTime()(a->sensorTime(), b->sensorTime()); }
  };
  typedef std::set<ObsDataPtr, lt_ObsDataPtr> DataSet;

  /*! Fetch all data for the given sensors within the time limits.
   */
  virtual DataSet allData(const std::vector<Sensor>& sensors, const TimeRange& limits) = 0;

  /*! Same as allData, but for a single sensor. */
  DataSet allData(const Sensor& sensor, const TimeRange& limits)
    { return allData(std::vector<Sensor>(1, sensor), limits); }

  /*! Find an observation.
   * \param st sensor and time to look at
   * \return an observation, or a 0-pointer if no observation is found
   */
  virtual ObsDataPtr find(const SensorTime& st) = 0;

  /*! Create an observation.
   * \param st sensor and time to look at
   * \return the new observation
   */
  virtual ObsDataPtr create(const SensorTime& st) = 0;

  /*! Apply updates.
   */
  virtual bool update(const std::vector<ObsUpdate>& updates) = 0;

public:
  /*! Different types of observation updates. */
  enum ObsDataChange {
    MODIFIED, // Observation has been modified
    CREATED,  // Observation appeared, either externally (new obs in kvalobs) or internally (created in the GUI)
    DESTROYED // Observation disappeared (pointer will become invalid)
  };

  /*! Signal an update to an observation. */
  boost::signal2<void, ObsDataChange, ObsDataPtr> obsDataChanged;
};

typedef std::shared_ptr<ObsAccess> ObsAccessPtr;

#endif // COMMON_OBSACCESS_HH
