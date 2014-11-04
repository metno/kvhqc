
#include "QueryTaskHelper.hh"

#include "DeleteTaskWhenDone.hh"
#include "QueryTaskHandler.hh"
#include "QueryTask.hh"
#include "SyncTask.hh"

#define MILOGGER_CATEGORY "kvhqc.QueryTaskHelper"
#include "util/HqcLogging.hh"

LOG_CONSTRUCT_COUNTER;

QueryTaskHelper::QueryTaskHelper(QueryTask* t)
  : mHandler(0)
  , mDeleter(new DeleteTaskWhenDone(t))
{
  METLIBS_LOG_SCOPE();
  LOG_CONSTRUCT();

  connect(task(), SIGNAL(taskDone(const QString&)), this, SLOT(onQueryDone()));
}

QueryTaskHelper::~QueryTaskHelper()
{
  METLIBS_LOG_SCOPE();
  LOG_DESTRUCT();
  drop();
}

void QueryTaskHelper::post(QueryTaskHandler_x handler, bool synchronized)
{
  METLIBS_LOG_SCOPE("this=" << this);
  assert(handler);
  assert(not mHandler);

  mHandler = handler;
  if (not synchronized)
    mHandler->postTask(task());
  else
    syncTask(task(), mHandler);
}

bool QueryTaskHelper::drop()
{
  METLIBS_LOG_SCOPE("this=" << this);
  const bool d = (not mDeleter) or (not mHandler) or mHandler->dropTask(task());
  mHandler = 0;

  mDeleter->enableDelete();
  mDeleter = 0;

  return d;
#if 0
  if (not mDeleter)
    return true;
  if (mHandler) {
    const bool dropped = mHandler->dropTask(task());
    mHandler = 0;
    if (not dropped) {
      METLIBS_LOG_DEBUG("task " << task() << " not dropped");
      disconnect(task(), SIGNAL(done()), this, SLOT(onQueryDone()));
      return false;
    }
  }

  mDeleter = 0;
  return true;
#endif
}

void QueryTaskHelper::onQueryDone()
{
  METLIBS_LOG_SCOPE("this=" << this);
  if (mHandler) {
    // important to set mHandler=0 before emitting the signal; else a
    // memory leak will occur if the destructor is called from a SLOT
    // (which calls drop, which calls mHandler->dropRequest, which
    // returns false as the task is done already, which creates a
    // DeleteTaskWhenDone that will never receive a done signal)
    mHandler = 0;

    Q_EMIT done(task()); // FIXME this is more like a callback; queued connections will not work
  }
}
