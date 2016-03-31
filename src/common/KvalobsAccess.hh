
#ifndef COMMON_KVALOBSACCESS_HH
#define COMMON_KVALOBSACCESS_HH 1

#include "KvBufferedAccess.hh"

#include "common/AbstractReinserter.hh"
#include "TimeRange.hh"

#include <kvcpp/kvservicetypes.h>

#include <boost/icl/interval_set.hpp>

/*! Access to data from a kvalobs database.
 * No listening for external updates.
 */
class KvalobsAccess : public KvBufferedAccess {
public:
  KvalobsAccess();
  ~KvalobsAccess();

  virtual TimeSet allTimes(const std::vector<Sensor>& sensors, const TimeRange& limits);
  virtual DataSet allData(const std::vector<Sensor>& sensors, const TimeRange& limits);
  using KvBufferedAccess::allTimes;
  using KvBufferedAccess::allData;

  virtual ObsDataPtr find(const SensorTime& st);
  virtual bool update(const std::vector<ObsUpdate>& updates);

  void nextData(kvservice::KvObsDataList &dl, bool update);

  void setReinserter(AbstractReinserterPtr reinserter)
    { mReinserter = reinserter; }
  AbstractReinserterPtr getReinserter() const
    { return mReinserter; }
  bool hasReinserter() const
    { return (mReinserter.get() != 0); }

  /*! Emitted when fetching data. The first parameter is number of
   *  stations to fetch data for, where fetching has finished. The
   *  second parameter is the number of stations finished so far. */
  boost::signal2<void, int, int> signalFetchingData;

protected:
  virtual bool drop(const SensorTime& st);
  virtual void findRange(const std::vector<Sensor>& sensors, const TimeRange& limits);

private:
  bool isFetched(int stationid, const timeutil::ptime& t) const;
  void addFetched(int stationid, const TimeRange& t);
  void removeFetched(int stationid, const timeutil::ptime& t);

  void findRange(const Sensor& sensor, const TimeRange& limits)
    { findRange(std::vector<Sensor>(1, sensor), limits); }

private:
  typedef boost::icl::interval_set<timeutil::ptime> FetchedTimes_t;
  typedef std::map<int, FetchedTimes_t> Fetched_t;
  Fetched_t mFetched;

  AbstractReinserterPtr mReinserter;

  int mCountHoursToFetch;
  int mCountFetchedHours;
  int mLastFetchedStationId;
  int mLastFetchedObsHour;
};
typedef std::shared_ptr<KvalobsAccess> KvalobsAccessPtr;

#endif // COMMON_KVALOBSACCESS_HH
