
#ifndef SIGNALTASK_HH
#define SIGNALTASK_HH 1

#include "QueryTask.hh"
#include <QtCore/QObject>

class SignalTask : public QObject, public QueryTask
{ Q_OBJECT;
public:
  SignalTask(QueryTask* wrapped);
  ~SignalTask();
  
  QString querySql(QString dbversion) const
    { return mWrapped->querySql(dbversion); }

  void notifyRow(const ResultRow& row)
    { mWrapped->notifyRow(row); }

  void notifyDone();
  void notifyError(QString message);

Q_SIGNALS:
  void signalDone();
  void signalError(QString message);

private:
  QueryTask* mWrapped;
};

#endif // SIGNALTASK_HH
