
#ifndef AnalyseRR24_hh
#define AnalyseRR24_hh 1

#include "EditAccess.hh"
#include "TimeSpan.hh"

namespace RR24 {
typedef std::vector<float> float_v;

bool analyse(EditAccessPtr da, const Sensor& sensor, TimeSpan& time);
void markPreviousAccumulation(EditAccessPtr da, const Sensor& sensor, const TimeSpan& time, bool before);
void redistribute(EditAccessPtr da, const Sensor& sensor, const timeutil::ptime& t0, const TimeSpan& editableTime,
    const float_v& newCorr);

bool redistributeProposal(EditAccessPtr da, const Sensor& sensor, const TimeSpan& time, float_v& values);

bool canRedistributeInQC2(EditAccessPtr da, const Sensor& sensor, const TimeSpan& time);
void redistributeInQC2(EditAccessPtr da, const Sensor& sensor,
    const TimeSpan& time, const TimeSpan& editableTime);

enum { AR_NONE = 0, AR_ACCEPT = 1, AR_REJECT = 2 };
void singles(EditAccessPtr da, const Sensor& sensor, const timeutil::ptime& t0, const TimeSpan& editableTime,
    const float_v& newCorrected, const std::vector<int>& acceptReject);

float calculateSum(EditAccessPtr da, const Sensor& sensor, const TimeSpan& time);

/**
   Calculate original sum before redistribution.
   @return sum, or MISSING in case it cannot be calculated
*/
float calculateOriginalSum(EditAccessPtr da, const Sensor& sensor, const TimeSpan& time);

bool canAccept(EditAccessPtr da, const Sensor& sensor, const TimeSpan& time);
void accept(EditAccessPtr da, const Sensor& sensor, const TimeSpan& time);
} // namespace RR24

#endif // AnalyseRR24_hh
