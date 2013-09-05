
#ifndef AnalyseErrors_hh
#define AnalyseErrors_hh 1

#include "EditAccess.hh"
#include "TimeRange.hh"

namespace Errors {

struct ErrorInfo {
    EditDataPtr obs;
    int flg, flTyp;
    bool fixed;
    ErrorInfo(EditDataPtr o = EditDataPtr()) : obs(o), flg(-1), flTyp(-1), fixed(false) { }
};

typedef std::vector<Sensor> Sensors_t;
typedef std::vector<ErrorInfo> Errors_t;

bool recheck(ErrorInfo& ei, bool errorsForSalen);
Errors_t fillMemoryStore2(EditAccessPtr eda, const Sensors_t& sensors, const TimeRange& timerange, bool errorsForSalen);

} // namespace Errors

#endif // AnalyseErrors_hh
