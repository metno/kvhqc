
#include "ObsAccess.hh"

ObsAccess::~ObsAccess()
{
}

void ObsAccess::addAllTimes(TimeSet& times, const std::vector<Sensor>& sensors, const TimeRange& limits)
{
    const TimeSet t = allTimes(sensors, limits);
    times.insert(t.begin(), t.end());
}
