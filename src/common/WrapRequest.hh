
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
  
  void completed(const QString& withError) Q_DECL_OVERRIDE;

  const Sensor_s& sensors() const Q_DECL_OVERRIDE
    { return mWrapped->sensors(); }

  const TimeSpan& timeSpan() const Q_DECL_OVERRIDE
    { return mWrapped->timeSpan(); }

  ObsFilter_p filter() const Q_DECL_OVERRIDE
    { return mWrapped->filter(); }

  ObsRequest_p wrapped() const
    { return mWrapped; }

private:
  ObsRequest_p mWrapped;
};

#endif // ACCESS_WRAPREQUEST_HH
