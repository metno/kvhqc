
#ifndef AnalyseRR24_hh
#define AnalyseRR24_hh 1

#include "TaskAccess.hh"
#include "common/TimeSpan.hh"

class ObsPgmRequest;

namespace RR24 {
typedef std::vector<float> float_v;

bool analyse(TaskAccess_p da, const Sensor& sensor, TimeSpan& time);
void markPreviousAccumulation(TaskAccess_p da, const Sensor& sensor, const TimeSpan& time, bool before);
void redistribute(TaskAccess_p da, const Sensor& sensor, const timeutil::ptime& t0, const TimeSpan& editableTime,
    const float_v& newCorr);

bool redistributeProposal(TaskAccess_p da, const Sensor& sensor, const TimeSpan& time, const ObsPgmRequest* op, float_v& values);

bool canRedistributeInQC2(TaskAccess_p da, const Sensor& sensor, const TimeSpan& time);
void redistributeInQC2(TaskAccess_p da, const Sensor& sensor,
    const TimeSpan& time, const TimeSpan& editableTime);

enum { AR_NONE = 0, AR_ACCEPT = 1, AR_REJECT = 2 };
void singles(TaskAccess_p da, const Sensor& sensor, const timeutil::ptime& t0, const TimeSpan& editableTime,
    const float_v& newCorrected, const std::vector<int>& acceptReject);

float calculateSum(TaskAccess_p da, const Sensor& sensor, const TimeSpan& time);

/**
   Calculate original sum before redistribution.
   @return sum, or MISSING in case it cannot be calculated
*/
float calculateOriginalSum(TaskAccess_p da, const Sensor& sensor, const TimeSpan& time);

bool canAccept(TaskAccess_p da, const Sensor& sensor, const TimeSpan& time);
void accept(TaskAccess_p da, const Sensor& sensor, const TimeSpan& time);
} // namespace RR24

#endif // AnalyseRR24_hh
