
#ifndef ACCESS_SIGNALREQUEST_HH
#define ACCESS_SIGNALREQUEST_HH 1

#include "WrapRequest.hh"

class SignalRequest : public WrapRequest
{ Q_OBJECT;

public:
  SignalRequest(const Sensor_s& sensors, const TimeSpan& timeSpan, ObsFilter_p filter = ObsFilter_p());
  SignalRequest(ObsRequest_p wrapped);
  
public:
  virtual void newData(const ObsData_pv& data);
  virtual void updateData(const ObsData_pv& data);
  virtual void dropData(const SensorTime_v& dropped);

Q_SIGNALS:
  void requestNewData(const ObsData_pv& data);
  void requestUpdateData(const ObsData_pv& data);
  void requestDropData(const SensorTime_v& dropped);
};

HQC_TYPEDEF_P(SignalRequest);

#endif // ACCESS_SIGNALREQUEST_HH
