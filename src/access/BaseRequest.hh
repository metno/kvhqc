

#ifndef ACCESS_BASEREQUEST_HH
#define ACCESS_BASEREQUEST_HH 1

#include "ObsRequest.hh"

class BaseRequest : public ObsRequest
{
public:
  BaseRequest(const Sensor_s& sensors, const TimeSpan& timeSpan, ObsFilter_p filter = ObsFilter_p());
  
  virtual const Sensor_s& sensors() const
    { return mSensors; }

  virtual const TimeSpan& timeSpan() const
    { return mTimeSpan; }

  virtual ObsFilter_p filter() const
    { return mFilter; }

private:
  Sensor_s mSensors;
  TimeSpan mTimeSpan;
  ObsFilter_p mFilter;
};

HQC_TYPEDEF_X(BaseRequest);

#endif // ACCESS_BASEREQUEST_HH
