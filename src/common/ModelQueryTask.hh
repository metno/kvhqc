
#ifndef COMMON_MODELQUERYTASK_HH
#define COMMON_MODELQUERYTASK_HH 1

#include "ModelData.hh"
#include "QueryTask.hh"

#include <QtCore/QObject>

class ModelQueryTask : public QObject, public QueryTask
{ Q_OBJECT;
public:
  ModelQueryTask(const SensorTime_v& sensorTimes, size_t priority);
  
  QString querySql(QString dbversion) const;
  void notifyRow(const ResultRow& row);
  void notifyStatus(int status);

Q_SIGNALS:
  void data(const ModelData_pv&);
  void queryStatus(int);

private:
  void sendData();

private:
  SensorTime_v mSensorTimes;
  ModelData_pv mData;
};

#endif // COMMON_MODELQUERYTASK_HH
