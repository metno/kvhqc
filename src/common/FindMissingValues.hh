
#ifndef common_FindMissingValues_hh
#define common_FindMissingValues_hh 1

#include "Sensor.hh"
#include "TimeRange.hh"

#include <set>
#include <vector>

namespace Missing {

/*! Search for missing observations.
 *
 * \param typeIds typeId contraint, or no contraint if empty
 * \param tLimits time range, only date is used
 * \return list of missing observations
 */
std::vector<SensorTime> find(const std::vector<int>& typeIds, const TimeRange& tLimits);

} // namespace Missing

#endif // common_FindMissingValues_hh
