
#ifndef COMMON_MODELQUERYTASK_HH
#define COMMON_MODELQUERYTASK_HH 1

#include "ModelData.hh"
#include "QueryTask.hh"

class ModelQueryTask : public QueryTask
{ Q_OBJECT;
public:
  ModelQueryTask(const SensorTime_v& sensorTimes, size_t priority);
  ~ModelQueryTask();
  
  QString querySql(QString dbversion) const;
  void notifyRow(const ResultRow& row);
  void notifyDone(const QString& withError);

Q_SIGNALS:
  void data(const ModelData_pv&);

private:
  void sendData();

private:
  SensorTime_v mSensorTimes;
  ModelData_pv mData;
};

#endif // COMMON_MODELQUERYTASK_HH
