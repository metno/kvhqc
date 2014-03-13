
#include "BaseRequest.hh"

BaseRequest::BaseRequest(const Sensor& sensor, const TimeSpan& timeSpan, ObsFilter_p filter)
  : mSensor(sensor)
  , mTimeSpan(timeSpan)
  , mFilter(filter)
{
}
