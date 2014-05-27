
#ifndef ACCESS_MODELREQUEST_HH
#define ACCESS_MODELREQUEST_HH 1

#include "ModelData.hh"
#include "util/TaggedObject.hh"
#include <QtCore/QObject>

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
  void notifyStatus(int status);

Q_SIGNALS:
  void data(const ModelData_pv&);
  void completed(bool);
  
private:
  SensorTime_v mSensorTimes;
};

HQC_TYPEDEF_P(ModelRequest);
HQC_TYPEDEF_PV(ModelRequest);

#endif // ACCESS_MODELREQUEST_HH
