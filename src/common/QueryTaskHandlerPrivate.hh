
#ifndef COMMON_QUERYTASKHANDLERPRIVATE_HH
#define COMMON_QUERYTASKHANDLERPRIVATE_HH 1

#include "QueryTaskHandler.hh"

#include "QueryTask.hh"
#include "util/priority_list.hh"

#include <QtCore/QMutex>
#include <QtCore/QThread>
#include <QtCore/QWaitCondition>

// ========================================================================

class QueryTaskThread : public QThread
{
public:
  QueryTaskThread(QueryTaskRunner_p runner);
  ~QueryTaskThread();

  void enqueueTask(QueryTask* task);
  void unqueueTask(QueryTask* task);

  void run();

private:
  QueryTaskRunner_p mRunner;

  typedef priority_list<QueryTask_x, QueryTask_by_Priority> Queue_t;
  Queue_t mQueue;

  QMutex mMutex;
  QWaitCondition mCondition;
  bool mDone;
};

#endif // COMMON_QUERYTASKHANDLERPRIVATE_HH
