
#ifndef HQC_VIEWCHANGES_HH
#define HQC_VIEWCHANGES_HH 1

#include "common/TimeSpan.hh"
#include <string>

class Sensor;
class SensorTime;

namespace ViewChanges {
void store(const Sensor& s, const std::string& vtype, const std::string& vid, const std::string& vchanges);
void forget(const Sensor& s, const std::string& vtype, const std::string& vid);
std::string fetch(const Sensor& s, const std::string& vtype, const std::string& vid);

TimeSpan defaultTimeLimits(const SensorTime& st);

} // namespace ViewChanges

#endif // HQC_VIEWCHANGES_HH
