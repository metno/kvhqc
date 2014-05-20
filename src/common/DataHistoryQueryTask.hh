
#ifndef COMMON_DATAHISTORYQUERYTASK_HH
#define COMMON_DATAHISTORYQUERYTASK_HH 1

#include "kvDataHistoryValues.hh"
#include "QueryTask.hh"
#include "Sensor.hh"

#include <QtCore/QObject>

class DataHistoryQueryTask : public QObject, public QueryTask
{ Q_OBJECT;
public:
  DataHistoryQueryTask(const SensorTime& st, size_t priority, QObject* parent=0);
  
  QString querySql(QString dbversion) const;
  void notifyRow(const ResultRow& row);
  void notifyDone();
  void notifyError(QString message);

Q_SIGNALS:
  void completed(const SensorTime&, const kvDataHistoryValues_v&, bool);

private:
  SensorTime mSensorTime;
  kvDataHistoryValues_v mHistory;
};

#endif // COMMON_DATAHISTORYQUERYTASK_HH
