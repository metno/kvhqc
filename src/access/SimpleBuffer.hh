
#ifndef ACCESS_SIMPLEBUFFER_HH
#define ACCESS_SIMPLEBUFFER_HH 1

#include "ObsAccess.hh"
#include "ObsFilter.hh"
#include "SignalRequest.hh"

class SimpleRequest;
HQC_TYPEDEF_P(SimpleRequest);

class SimpleBuffer : public QObject
{ Q_OBJECT;

public:
  SimpleBuffer(const Sensor& sensor, const TimeSpan& timeSpan, ObsFilter_p filter);
  SimpleBuffer(SignalRequest_p mRequest);
  virtual ~SimpleBuffer() = 0;

  void postRequest(ObsAccess_p access);

  void syncRequest(ObsAccess_p access);

  enum CompleteStatus { INCOMPLETE, FAILED, COMPLETE };
  
  CompleteStatus status() const
    { return mComplete; }

  ObsRequest_p request() const
    { return mRequest; }

public Q_SLOTS:
  virtual void completed(bool failed);
  virtual void newData(const ObsData_pv& data) = 0;
  virtual void updateData(const ObsData_pv& data) = 0;
  virtual void dropData(const SensorTime_v& dropped) = 0;

private:
  ObsRequest_p mRequest;
  ObsAccess_p mAccess;
  CompleteStatus mComplete;
};

HQC_TYPEDEF_P(SimpleBuffer);
HQC_TYPEDEF_X(SimpleBuffer);

#endif // ACCESS_SIMPLEBUFFER_HH
