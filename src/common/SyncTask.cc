
#include "SyncTask.hh"

#include "QueryTask.hh"
#include "util/Synchronizer.hh"

namespace {
class SyncedTask : public QueryTask
{
public:
  SyncedTask(QueryTask* q)
    : QueryTask(q->priority()), mTask(q) { }

  ~SyncedTask();

  void run(QueryTaskHandler* handler);

  QString querySql(QString dbversion) const override { return mTask->querySql(dbversion); }

  void notifyRow(const ResultRow& row) override { mTask->notifyRow(row); }

  void notifyDone(const QString& withError) override;

  int remaining() override { return mTask->remaining(); }

private:
  QSemaphore mSemaphore;
  QueryTask* mTask;
  QString mWithError;
};

SyncedTask::~SyncedTask()
{
}

void SyncedTask::run(QueryTaskHandler* handler)
{
  handler->postTask(this);
  mSemaphore.acquire();
  mTask->notifyDone(mWithError);
}
  
void SyncedTask::notifyDone(const QString& withError)
{
  mWithError = withError;
  mSemaphore.release();
}

} // anonymous namespace

QueryTask* syncTask(QueryTask* task, QueryTaskHandler* handler)
{
#if 0
  Synchronizer sync;
  QObject::connect(task, SIGNAL(taskDone(const QString&)), &sync, SLOT(taskDone()));

  handler->postTask(task);
  sync.waitForSignal();
#else
  SyncedTask sync(task);
  sync.run(handler);
#endif

  return task;
}
