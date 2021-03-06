
#ifndef AnalyseRR24_hh
#define AnalyseRR24_hh 1

#include "EditAccess.hh"
#include "TimeRange.hh"

namespace RR24 {
typedef std::vector<float> float_v;

bool analyse(EditAccessPtr da, const Sensor& sensor, TimeRange& time);
void markPreviousAccumulation(EditAccessPtr da, const Sensor& sensor, const TimeRange& time, bool before);
void redistribute(EditAccessPtr da, const Sensor& sensor, const timeutil::ptime& t0, const TimeRange& editableTime,
    const float_v& newCorr);

bool redistributeProposal(EditAccessPtr da, const Sensor& sensor, const TimeRange& time, float_v& values);

bool canRedistributeInQC2(EditAccessPtr da, const Sensor& sensor, const TimeRange& time);
void redistributeInQC2(EditAccessPtr da, const Sensor& sensor,
    const TimeRange& time, const TimeRange& editableTime);

enum { AR_NONE = 0, AR_ACCEPT = 1, AR_REJECT = 2 };
void singles(EditAccessPtr da, const Sensor& sensor, const timeutil::ptime& t0, const TimeRange& editableTime,
    const float_v& newCorrected, const std::vector<int>& acceptReject);

float calculateSum(EditAccessPtr da, const Sensor& sensor, const TimeRange& time);

/**
   Calculate original sum before redistribution.
   @return sum, or MISSING in case it cannot be calculated
*/
float calculateOriginalSum(EditAccessPtr da, const Sensor& sensor, const TimeRange& time);

bool canAccept(EditAccessPtr da, const Sensor& sensor, const TimeRange& time);
void accept(EditAccessPtr da, const Sensor& sensor, const TimeRange& time);
} // namespace RR24

#endif // AnalyseRR24_hh
