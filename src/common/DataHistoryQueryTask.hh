
#ifndef COMMON_DATAHISTORYQUERYTASK_HH
#define COMMON_DATAHISTORYQUERYTASK_HH 1

#include "kvDataHistoryValues.hh"
#include "QueryTask.hh"
#include "Sensor.hh"

class DataHistoryQueryTask : public QueryTask
{ Q_OBJECT;
public:
  DataHistoryQueryTask(const SensorTime& st, size_t priority);
  
  QString querySql(QString dbversion) const;
  void notifyRow(const ResultRow& row);
  void notifyDone(const QString& withError);
  
  const SensorTime& sensorTime() const
    { return mSensorTime; }

  const kvDataHistoryValues_v& history() const
    { return mHistory; }

  kvDataHistoryValues_v& history()
    { return mHistory; }

private:
  SensorTime mSensorTime;
  kvDataHistoryValues_v mHistory;
};

#endif // COMMON_DATAHISTORYQUERYTASK_HH
