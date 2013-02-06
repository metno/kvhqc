
#ifndef KvalobsAccess_hh
#define KvalobsAccess_hh 1

#include "KvBufferedAccess.hh"
#include "TimeRange.hh"

#include <decodeutility/DataReinserter.h>
#include <kvcpp/KvApp.h>
#include <kvcpp/kvservicetypes.h>

class KvalobsAccess : public KvBufferedAccess {
public:
    KvalobsAccess();
    ~KvalobsAccess();

    virtual ObsDataPtr find(const SensorTime& st);
    virtual bool update(const std::vector<ObsUpdate>& updates);

    void nextData(kvservice::KvObsDataList &dl);

    typedef kvalobs::DataReinserter<kvservice::KvApp> Reinserter_t;
    void setReinserter(Reinserter_t* reinserter)
        { mDataReinserter = reinserter; }
    bool hasReinserter() const
        { return (mDataReinserter != 0); }

protected:
    virtual bool drop(const SensorTime& st);

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

    Reinserter_t* mDataReinserter;
};
typedef boost::shared_ptr<KvalobsAccess> KvalobsAccessPtr;

#endif // KvalobsAccess_hh
