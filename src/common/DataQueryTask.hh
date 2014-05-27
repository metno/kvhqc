
#ifndef COMMON_DATAQUERYTASK_HH
#define COMMON_DATAQUERYTASK_HH 1

#include "QueryTask.hh"
#include "ObsRequest.hh"

#include <QtCore/QObject>

class DataQueryTask : public QObject, public QueryTask
{ Q_OBJECT;
public:
  DataQueryTask(ObsRequest_p request, size_t priority);
  
  QString querySql(QString dbversion) const;
  void notifyRow(const ResultRow& row);
  void notifyStatus(int);
  void notifyError(QString message);

Q_SIGNALS:
  void newData(ObsRequest_p, const ObsData_pv&);
  void queryStatus(ObsRequest_p, int);

private:
  void sendData();

private:
  ObsRequest_p mRequest;
  ObsData_pv mData;
};

#endif // COMMON_DATAQUERYTASK_HH
