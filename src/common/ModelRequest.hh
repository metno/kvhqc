
#ifndef ACCESS_MODELREQUEST_HH
#define ACCESS_MODELREQUEST_HH 1

#include "ModelData.hh"
#include "util/TaggedObject.hh"
#include <QObject>

class ModelRequest : public QObject, public TaggedObject
{ Q_OBJECT;
public:
  ModelRequest(const SensorTime_v& sensorTimes)
    : mSensorTimes(sensorTimes) { }
  
  ~ModelRequest() { }
  
  const SensorTime_v& sensorTimes() const
    { return mSensorTimes; }

public Q_SLOTS:
  void notifyData(const ModelData_pv&);
  void notifyDone(const QString& withError);

Q_SIGNALS:
  void data(const ModelData_pv&);
  void requestCompleted(const QString& withError);
  
private:
  SensorTime_v mSensorTimes;
};

HQC_TYPEDEF_P(ModelRequest);
HQC_TYPEDEF_PV(ModelRequest);

#endif // ACCESS_MODELREQUEST_HH
