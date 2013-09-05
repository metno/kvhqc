
#ifndef AnalyseErrors_hh
#define AnalyseErrors_hh 1

#include "errorlist.h" // for struct ErrorList::mem
#include "TimeRange.hh"

namespace Errors {

std::vector<ErrorList::mem> fillMemoryStore2(const std::vector<int>& selectedParameters,
                                             const TimeRange& timerange,
                                             bool errorsForSalen,
                                             model::KvalobsDataListPtr dtl,
                                             const std::vector<modDatl>& mdtl);

} // namespace Errors

#endif // AnalyseErrors_hh
