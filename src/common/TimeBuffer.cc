
#include "TimeBuffer.hh"

TimeBuffer::TimeBuffer(const Sensor_s& sensors, const TimeSpan& timeSpan, ObsFilter_p filter)
  : SortedBuffer(std::make_shared<TimeBuffer::Ordering>(), sensors, timeSpan, filter)
{
}

TimeBuffer::TimeBuffer(SignalRequest_p request)
  : SortedBuffer(std::make_shared<TimeBuffer::Ordering>(), request)
{
}
