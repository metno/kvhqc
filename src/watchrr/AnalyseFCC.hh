
#ifndef AnalyseFCC_hh
#define AnalyseFCC_hh 1

#include "TaskAccess.hh"
#include "common/TimeSpan.hh"

namespace FCC {

void analyse(TaskAccess_p da, const Sensor& sensor, const TimeSpan& time);
void acceptRow(TaskAccess_p da, const Sensor& sensor, const timeutil::ptime& time);

} // namespace FCC

#endif // AnalyseFCC_hh
