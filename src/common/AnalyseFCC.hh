
#ifndef AnalyseFCC_hh
#define AnalyseFCC_hh 1

#include "EditAccess.hh"
#include "TimeRange.hh"

namespace FCC {

void analyse(EditAccessPtr da, const Sensor& sensor, const TimeRange& time);
void acceptRow(EditAccessPtr da, const Sensor& sensor, const timeutil::ptime& time);

} // namespace FCC

#endif // AnalyseFCC_hh
