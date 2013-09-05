
#ifndef ViewChanges_hh
#define ViewChanges_hh 1

#include "TimeRange.hh"
#include <string>

class Sensor;
class SensorTime;

namespace ViewChanges {
void store(const Sensor& s, const std::string& vtype, const std::string& vid, const std::string& vchanges);
std::string fetch(const Sensor& s, const std::string& vtype, const std::string& vid);

TimeRange defaultTimeLimits(const SensorTime& st);

} // namespace ViewChanges

#endif // ViewChanges_hh
