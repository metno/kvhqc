
#include "TimeBuffer.hh"

#include <boost/make_shared.hpp>

TimeBuffer::TimeBuffer(const Sensor_s& sensors, const TimeSpan& timeSpan, ObsFilter_p filter)
  : SortedBuffer(boost::make_shared<TimeBuffer::Ordering>(), sensors, timeSpan, filter)
{
}

TimeBuffer::TimeBuffer(SignalRequest_p request)
  : SortedBuffer(boost::make_shared<TimeBuffer::Ordering>(), request)
{
}
