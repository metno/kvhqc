
#ifndef COMMON_DATAQUERYTASK_HH
#define COMMON_DATAQUERYTASK_HH 1

#include "KvalobsData.hh"
#include "QueryTask.hh"
#include "ObsRequest.hh"

#include <QtCore/QObject>

class KvalobsDataRow {
public:
  QString columns(QString data_alias);
  int columnCount();
  KvalobsData_p extract(const ResultRow& row, int col=0);
};

// ========================================================================

class DataQueryTask : public QueryTask
{ Q_OBJECT;
public:
  DataQueryTask(ObsRequest_p request, size_t priority);
  
  QString querySql(QString dbversion) const;
  void notifyRow(const ResultRow& row);
  void notifyDone(const QString& withError);

Q_SIGNALS:
  void newData(ObsRequest_p, const ObsData_pv&);
  void queryDone(ObsRequest_p, const QString&);

private:
  void sendData();

private:
  ObsRequest_p mRequest;
  ObsData_pv mData;
};

#endif // COMMON_DATAQUERYTASK_HH
