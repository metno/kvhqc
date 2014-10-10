
#ifndef ACCESS_WRAPREQUEST_HH
#define ACCESS_WRAPREQUEST_HH 1

#include "ObsRequest.hh"

class WrapRequest;
HQC_TYPEDEF_X(WrapRequest);
HQC_TYPEDEF_P(WrapRequest);

class WrapRequest : public ObsRequest
{
public:
  WrapRequest(ObsRequest_p wrapped);
  ~WrapRequest();

  static WrapRequest_p untag(ObsRequest_p wrapped);
  
  virtual void completed(const QString& withError);

  virtual const Sensor_s& sensors() const
    { return mWrapped->sensors(); }

  virtual const TimeSpan& timeSpan() const
    { return mWrapped->timeSpan(); }

  virtual ObsFilter_p filter() const
    { return mWrapped->filter(); }

  ObsRequest_p wrapped() const
    { return mWrapped; }

private:
  ObsRequest_p mWrapped;
};

#endif // ACCESS_WRAPREQUEST_HH
