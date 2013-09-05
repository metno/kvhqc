
#ifndef FindExtremeValues_hh
#define FindExtremeValues_hh 1

#include "Sensor.hh"
#include "TimeRange.hh"

#include <vector>

namespace Extremes {

std::vector<SensorTime> find(int paramid, const TimeRange& tLimits);

} // namespace Extremes

#endif // FindExtremeValues_hh
