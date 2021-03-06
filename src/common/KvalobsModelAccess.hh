
#ifndef KvalobsModelAccess_hh
#define KvalobsModelAccess_hh 1

#include "KvModelAccess.hh"
#include "TimeRange.hh"

#include <kvcpp/kvservicetypes.h>
#include <boost/icl/interval_set.hpp>

class KvalobsModelAccess : public KvModelAccess {
public:
  KvalobsModelAccess();
  ~KvalobsModelAccess();

  virtual ModelDataSet findMany(const std::vector<SensorTime>& sensorTimes);
  virtual ModelDataSet allData(const std::vector<Sensor>& sensors, const TimeRange& limits);

private:
  bool isFetched(int stationid, const timeutil::ptime& t) const;
  void addFetched(int stationid, const TimeRange& t);
  void removeFetched(int stationid, const timeutil::ptime& t);

private:
  typedef boost::icl::interval_set<timeutil::ptime> FetchedTimes_t;
  typedef std::map<int, FetchedTimes_t> Fetched_t;
  Fetched_t mFetched;
};

typedef std::shared_ptr<KvalobsModelAccess> KvalobsModelAccessPtr;

#endif // KvalobsModelAccess_hh
