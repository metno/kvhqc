
#ifndef COMMON_KVALOBSACCESS_HH
#define COMMON_KVALOBSACCESS_HH 1

#include "KvBufferedAccess.hh"
#include "TimeRange.hh"

#include <decodeutility/DataReinserter.h>
#include <kvcpp/KvApp.h>
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

  typedef kvalobs::DataReinserter<kvservice::KvApp> Reinserter_t;
  void setReinserter(Reinserter_t* reinserter)
    { mDataReinserter.reset(reinserter); }
  bool hasReinserter() const
    { return (mDataReinserter.get() != 0); }

  /*! Emitted when fetching data. The first parameter is number of
   *  stations to fetch data for, where fetching has finished. The
   *  second parameter is the number of stations finished so far. */
  boost::signal2<void, int, int> signalFetchingData;
  
protected:
  virtual bool drop(const SensorTime& st);

  virtual void newStationWithData(int stationId);

private:
  bool isFetched(int stationid, const timeutil::ptime& t) const;
  void addFetched(int stationid, const TimeRange& t);
  void removeFetched(int stationid, const timeutil::ptime& t);
  
  void findRange(const Sensor& sensor, const TimeRange& limits)
    { findRange(std::vector<Sensor>(1, sensor), limits); }
  void findRange(const std::vector<Sensor>& sensors, const TimeRange& limits);
  
protected:
  typedef std::set<int> stations_with_data_t;
  stations_with_data_t mStationsWithData;

private:
  typedef boost::icl::interval_set<timeutil::ptime> FetchedTimes_t;
  typedef std::map<int, FetchedTimes_t> Fetched_t;
  Fetched_t mFetched;
  
  std::auto_ptr<Reinserter_t> mDataReinserter;

  int mCountStationsToFetch;
  int mCountFetchedStations;
  int mLastFetchedStationId;
};
typedef boost::shared_ptr<KvalobsAccess> KvalobsAccessPtr;

#endif // COMMON_KVALOBSACCESS_HH
