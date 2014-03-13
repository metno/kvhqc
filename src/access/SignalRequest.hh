
#ifndef ACCESS_SIGNALREQUEST_HH
#define ACCESS_SIGNALREQUEST_HH 1

#include "BaseRequest.hh"

class SignalRequest : public QObject, public ObsRequest
{ Q_OBJECT;

public:
  SignalRequest(const Sensor& sensor, const TimeSpan& timeSpan, ObsFilter_p filter = ObsFilter_p());
  SignalRequest(ObsRequest_p wrapped);
  
public:
  virtual const Sensor& sensor() const
    { return mWrapped->sensor(); }

  virtual const TimeSpan& timeSpan() const
    { return mWrapped->timeSpan(); }

  virtual ObsFilter_p filter() const
    { return mWrapped->filter(); }

  virtual void completed(bool failed);
  virtual void newData(const ObsData_pv& data);
  virtual void updateData(const ObsData_pv& data);
  virtual void dropData(const SensorTime_v& dropped);

Q_SIGNALS:
  void requestCompleted(bool failed);
  void requestNewData(const ObsData_pv& data);
  void requestUpdateData(const ObsData_pv& data);
  void requestDropData(const SensorTime_v& dropped);

private:
  ObsRequest_p mWrapped;
};

HQC_TYPEDEF_P(SignalRequest);

#endif // ACCESS_SIGNALREQUEST_HH
