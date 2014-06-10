
#ifndef SIGNALTASK_HH
#define SIGNALTASK_HH 1

#include "QueryTask.hh"
#include <QtCore/QObject>

class SignalTask : public QObject, public QueryTask
{ Q_OBJECT;
public:
  SignalTask(size_t priority);

  void notifyStatus(int status);
  void notifyError(QString message);
  void notifyDone();
  
Q_SIGNALS:
  void status(int status);
  void error(QString message);
  void done();
};

// ########################################################################

class WrapperTask : public SignalTask
{ Q_OBJECT;
public:
  WrapperTask(QueryTask* wrapped);
  ~WrapperTask();
  
  QString querySql(QString dbversion) const;
  void notifyRow(const ResultRow& row);
  void notifyStatus(int status);
  void notifyError(QString message);
  void notifyDone();
  
private:
  QueryTask* mWrapped;
};

#endif // SIGNALTASK_HH
