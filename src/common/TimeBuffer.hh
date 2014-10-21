
#ifndef ACCESS_TIMEBUFFER_HH
#define ACCESS_TIMEBUFFER_HH 1

#include "SortedBuffer.hh"

class TimeBuffer : public SortedBuffer
{
public:
  class Ordering : public SortedBuffer::Ordering {
  public:
    bool compare(const SensorTime& a, const SensorTime& b) const
      { return lt_SensorTime()(a, b); }
  };

  TimeBuffer(const Sensor_s& sensors, const TimeSpan& timeSpan, ObsFilter_p filter = ObsFilter_p());
  TimeBuffer(SignalRequest_p request);
};

HQC_TYPEDEF_P(TimeBuffer);
HQC_TYPEDEF_PV(TimeBuffer);

#endif // ACCESS_TIMEBUFFER_HH
