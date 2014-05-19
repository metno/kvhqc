
#ifndef ACCESS_BACKGROUNDACCESSPRIVATE_HH
#define ACCESS_BACKGROUNDACCESSPRIVATE_HH 1

#include "BackgroundAccess.hh"

#include "QueryTask.hh"

#include <QtCore/QMutex>
#include <QtCore/QThread>
#include <QtCore/QWaitCondition>

#include <queue> // priority_queue

// ========================================================================

class BackgroundThread : public QThread
{ Q_OBJECT;
public:
  BackgroundThread(BackgroundHandler_p handler);
  ~BackgroundThread();

  void enqueueTask(QueryTask* task);

  virtual void run();

Q_SIGNALS:
  void newData(ObsRequest_p request, const ObsData_pv& data);

private:
  QueryTask* taskForRequest(ObsRequest_p request);

private:
  BackgroundHandler_p mHandler;

  typedef std::priority_queue<QueryTask_x, std::vector<QueryTask_x>, QueryTask_by_Priority> Queue_t;
  Queue_t mQueue;

  QMutex mMutex;
  QWaitCondition mCondition;
  bool mDone;
};

#endif // ACCESS_BACKGROUNDACCESSPRIVATE_HH
