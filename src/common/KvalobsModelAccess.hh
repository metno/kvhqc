
#ifndef KvalobsModelAccess_hh
#define KvalobsModelAccess_hh 1

#include "KvModelAccess.hh"
#include "TimeSpan.hh"

#include <kvcpp/kvservicetypes.h>
#include <boost/icl/interval_set.hpp>

class KvalobsModelAccess : public KvModelAccess {
public:
  KvalobsModelAccess();
  ~KvalobsModelAccess();

  virtual ModelDataSet findMany(const std::vector<SensorTime>& sensorTimes);
  virtual ModelDataSet allData(const std::vector<Sensor>& sensors, const TimeSpan& limits);

private:
  bool isFetched(int stationid, const timeutil::ptime& t) const;
  void addFetched(int stationid, const TimeSpan& t);
  void removeFetched(int stationid, const timeutil::ptime& t);

private:
  typedef boost::icl::interval_set<timeutil::ptime> FetchedTimes_t;
  typedef std::map<int, FetchedTimes_t> Fetched_t;
  Fetched_t mFetched;
};

typedef boost::shared_ptr<KvalobsModelAccess> KvalobsModelAccessPtr;

#endif // KvalobsModelAccess_hh
