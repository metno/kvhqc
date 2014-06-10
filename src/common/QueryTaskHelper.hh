
#ifndef COMMON_QUERYTASKHELPER_HH
#define COMMON_QUERYTASKHELPER_HH 1

#include <QObject>

class SignalTask;
class QueryTaskHandler;

class DeleteTaskWhenDone : public QObject
{ Q_OBJECT;
public:
  DeleteTaskWhenDone(SignalTask* t);

private Q_SLOTS:
  void onQueryDone();

private:
  SignalTask* mTask;
};

// ########################################################################

class QueryTaskHelper : public QObject
{ Q_OBJECT;

public:
  QueryTaskHelper(SignalTask* task = 0);
  ~QueryTaskHelper();

  void post(QueryTaskHandler* handler);
  bool drop();
             
private Q_SLOTS:
  void onQueryDone();

Q_SIGNALS:
  void done(SignalTask* task);

private:
  QueryTaskHandler* mHandler;
  SignalTask* mTask;
};

#endif // COMMON_QUERYTASKHELPER_HH
