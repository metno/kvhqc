
#include "BaseRequest.hh"

BaseRequest::BaseRequest(const Sensor_s& sensors, const TimeSpan& timeSpan, ObsFilter_p filter)
  : mSensors(sensors)
  , mTimeSpan(timeSpan)
  , mFilter(filter)
{
}
