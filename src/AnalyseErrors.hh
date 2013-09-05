
#ifndef AnalyseErrors_hh
#define AnalyseErrors_hh 1

#include "EditAccess.hh"
#include "errorlist.h" // for struct ErrorList::mem
#include "TimeRange.hh"

namespace Errors {

std::vector<ErrorList::mem> fillMemoryStore2(const std::vector<int>& selectedParameters,
                                             const TimeRange& timerange,
                                             bool errorsForSalen,
                                             model::KvalobsDataListPtr dtl,
                                             const std::vector<modDatl>& mdtl);

typedef std::vector<Sensor> Sensors_t;
typedef std::vector<ObsDataPtr> Errors_t;

Errors_t fillMemoryStore2(EditAccessPtr eda, const Sensors_t& sensors, const TimeRange& timerange, bool errorsForSalen);

} // namespace Errors

#endif // AnalyseErrors_hh
