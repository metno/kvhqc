
#ifndef COMMON_STATIONQUERYTASK_HH
#define COMMON_STATIONQUERYTASK_HH 1

#include "QueryTask.hh"
#include <kvalobs/kvStation.h>
#include <QtCore/QObject>
#include <vector>

class StationQueryTask : public QObject, public QueryTask
{ Q_OBJECT;
public:
  typedef std::vector<kvalobs::kvStation> kvStation_v;

  StationQueryTask(size_t priority);
  
  QString querySql(QString dbversion) const;
  void notifyRow(const ResultRow& row);
  void notifyStatus(int status);

  const kvStation_v& stations() const
    { return mStations; }

Q_SIGNALS:
  void queryStatus(int);

private:
  kvStation_v mStations;
};

#endif // COMMON_STATIONQUERYTASK_HH
