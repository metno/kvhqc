
#ifndef ObsSubscription_hh
#define ObsSubscription_hh 1

#include "TimeRange.hh"
//#include <boost/shared_ptr.hpp>

struct ObsSubscription {
    ObsSubscription(int stationId, const TimeRange& time)
        : mStationId(stationId), mTime(time) { }

    int stationId() const
        { return mStationId; }

    const TimeRange& time() const
        { return mTime; }

    int mStationId;
    TimeRange mTime;
};

//typedef boost::shared_ptr<ObsSubscription> ObsSubscriptionPtr;

#endif // ObsSubscription_hh
