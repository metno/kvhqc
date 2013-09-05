
#include "ObsAccess.hh"

ObsAccess::~ObsAccess()
{
}

void ObsAccess::addAllTimes(TimeSet& times, const Sensor& sensor, const TimeRange& limits)
{
    const TimeSet t = allTimes(sensor, limits);
    times.insert(t.begin(), t.end());
}
