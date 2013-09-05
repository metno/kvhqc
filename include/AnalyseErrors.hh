
#ifndef AnalyseErrors_hh
#define AnalyseErrors_hh 1

#include "EditAccess.hh"
#include "TimeRange.hh"

namespace Errors {

typedef std::vector<Sensor> Sensors_t;
typedef std::vector<EditDataPtr> Errors_t;

Errors_t fillMemoryStore2(EditAccessPtr eda, const Sensors_t& sensors, const TimeRange& timerange, bool errorsForSalen);

} // namespace Errors

#endif // AnalyseErrors_hh
