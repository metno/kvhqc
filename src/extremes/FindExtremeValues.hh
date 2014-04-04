
#ifndef FindExtremeValues_hh
#define FindExtremeValues_hh 1

#include "Sensor.hh"
#include "TimeSpan.hh"

#include <vector>

namespace Extremes {

std::vector<SensorTime> find(int paramid, const TimeSpan& tLimits);

} // namespace Extremes

#endif // FindExtremeValues_hh
