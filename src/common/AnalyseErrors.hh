
#ifndef AnalyseErrors_hh
#define AnalyseErrors_hh 1

#include "EditAccess.hh"
#include "TimeRange.hh"

namespace Errors {

struct ErrorInfo {
  enum { OK = 0, BAD_IN_ERRORLIST2012 = 1, BAD_IN_ERRORLIST2013 = 2 };
  EditDataPtr obs;
  int badInList;
  ErrorInfo(EditDataPtr o = EditDataPtr()) : obs(o), badInList(0) { }
};

typedef std::vector<Sensor> Sensors_t;
typedef std::vector<ErrorInfo> Errors_t;

bool recheck(ErrorInfo& ei, bool errorsForSalen);
Errors_t fillMemoryStore2(EditAccessPtr eda, const Sensors_t& sensors, const TimeRange& timerange, bool errorsForSalen);

} // namespace Errors

#endif // AnalyseErrors_hh
