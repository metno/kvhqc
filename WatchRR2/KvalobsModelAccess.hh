
#ifndef KvalobsModelAccess_hh
#define KvalobsModelAccess_hh 1

#include "KvModelAccess.hh"
#include "TimeRange.hh"

#include <kvcpp/kvservicetypes.h>

class KvalobsModelAccess : public KvModelAccess {
public:
  KvalobsModelAccess();
  ~KvalobsModelAccess();

  virtual ModelDataSet findMany(const std::vector<SensorTime>& sensorTimes);

private:
  struct Fetched {
    int stationId;
    timeutil::ptime time;
    Fetched(int s, const timeutil::ptime& t)
      : stationId(s), time(t) { }
    bool operator<(const Fetched& other) const
      { if (stationId != other.stationId) return stationId < other.stationId; else return time < other.time; }
  };
  typedef std::set<Fetched> Fetched_t;
  Fetched_t mFetched;

};
typedef boost::shared_ptr<KvalobsModelAccess> KvalobsModelAccessPtr;

#endif // KvalobsModelAccess_hh
