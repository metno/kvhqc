
#ifndef ACCESS_SIMPLEREQUEST_HH
#define ACCESS_SIMPLEREQUEST_HH 1

#include "BaseRequest.hh"
#include "SimpleBuffer.hh"

class SimpleRequest : public BaseRequest
{
public:
  SimpleRequest(SimpleBuffer* buffer, const Sensor& sensor, const TimeSpan& timeSpan, ObsFilter_p filter = ObsFilter_p());
  
  virtual void completed(bool failed);
  virtual void newData(const ObsData_pv& data);
  virtual void updateData(const ObsData_pv& data);
  virtual void dropData(const SensorTime_v& dropped);

private:
  SimpleBuffer* mBuffer;
};

HQC_TYPEDEF_X(SimpleRequest);

#endif // ACCESS_SIMPLEREQUEST_HH
