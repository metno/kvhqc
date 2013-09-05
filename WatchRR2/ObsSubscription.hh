
#ifndef ObsSubscription_hh
#define ObsSubscription_hh 1

#include "Sensor.hh"
#include "TimeRange.hh"

struct ObsSubscription {
    ObsSubscription(const Sensor& sensor, const TimeRange& time)
        : mStationId(sensor.stationId), mTime(time) { }

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
