
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
  SimpleBuffer(const Sensor_s& sensors, const TimeSpan& timeSpan, ObsFilter_p filter);
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
  virtual void completed(const QString& withError);
  void newData(const ObsData_pv& data);
  void updateData(const ObsData_pv& data);
  void dropData(const SensorTime_v& dropped);

Q_SIGNALS:
  void bufferCompleted(const QString& withError);
  void newDataBegin();
  void newDataEnd(const ObsData_pv& data);
  void updateDataBegin();
  void updateDataEnd(const ObsData_pv& data);
  void dropDataBegin();
  void dropDataEnd(const SensorTime_v& dropped);

protected:
  virtual void onNewData(const ObsData_pv& data) = 0;
  virtual void onUpdateData(const ObsData_pv& data) = 0;
  virtual void onDropData(const SensorTime_v& dropped) = 0;

private:
  ObsRequest_p mRequest;
  ObsAccess_p mAccess;
  CompleteStatus mComplete;
};

HQC_TYPEDEF_P(SimpleBuffer);
HQC_TYPEDEF_X(SimpleBuffer);

#endif // ACCESS_SIMPLEBUFFER_HH
