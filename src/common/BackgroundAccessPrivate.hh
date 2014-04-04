
#ifndef ACCESS_BACKGROUNDACCESSPRIVATE_HH
#define ACCESS_BACKGROUNDACCESSPRIVATE_HH 1

#include "BackgroundAccess.hh"

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

  void enqueueRequest(ObsRequest_p request);

  virtual void run();

Q_SIGNALS:
  void newData(ObsRequest_p request, const ObsData_pv& data);

private:
  struct QueuedQuery {
    int priority;

    ObsRequest_p request;

    QueuedQuery(int p, ObsRequest_p r)
      : priority(p), request(r) { }
    
    bool operator<(const QueuedQuery& other) const
      { return priority < other.priority; }
  };

  BackgroundHandler_p mHandler;

  typedef std::priority_queue<QueuedQuery> QueryQueue_t;
  QueryQueue_t mQueue;

  QMutex mMutex;
  QWaitCondition mCondition;
  bool mDone;
};

#endif // ACCESS_BACKGROUNDACCESSPRIVATE_HH
