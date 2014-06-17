
#include "QueryTaskHelper.hh"

#include "DeleteTaskWhenDone.hh"
#include "QueryTaskHandler.hh"
#include "SignalTask.hh"

#define MILOGGER_CATEGORY "kvhqc.QueryTaskHelper"
#include "util/HqcLogging.hh"

LOG_CONSTRUCT_COUNTER;

QueryTaskHelper::QueryTaskHelper(SignalTask* task)
  : mHandler(0)
  , mTask(task)
{
  METLIBS_LOG_SCOPE();
  LOG_CONSTRUCT();
}

QueryTaskHelper::~QueryTaskHelper()
{
  METLIBS_LOG_SCOPE();
  LOG_DESTRUCT();
  if (not drop()) {
    // posted but not yet done; cannot delete task before done was
    // emitted (as QueryTaskHandler still makes calls to it), so we
    // create an object to wait for the done signal
    new DeleteTaskWhenDone(mTask);
  }
}

void QueryTaskHelper::post(QueryTaskHandler* handler)
{
  METLIBS_LOG_SCOPE("this=" << this);
  if (not mTask)
    return;
  assert(handler);
  assert(not mHandler);
  connect(mTask, SIGNAL(done()), this, SLOT(onQueryDone()));
  mHandler = handler;
  mHandler->postTask(mTask);
}

bool QueryTaskHelper::drop()
{
  METLIBS_LOG_SCOPE("this=" << this);
  if (not mTask)
    return true;
  if (mHandler) {
    const bool dropped = mHandler->dropTask(mTask);
    mHandler = 0;
    if (not dropped) {
      METLIBS_LOG_DEBUG("task " << mTask << " not dropped");
      disconnect(mTask, SIGNAL(done()), this, SLOT(onQueryDone()));
      return false;
    }
  }

  delete mTask;
  mTask = 0;
  return true;
}

void QueryTaskHelper::onQueryDone()
{
  METLIBS_LOG_SCOPE("this=" << this);
  if (mHandler and mTask) {
    // important to set mHandler=0 before emitting the signal; else a
    // memory leak will occur if the destructor is called from a SLOT
    // (which calls drop, which calls mHandler->dropRequest, which
    // returns false as the task is done already, which creates a
    // DeleteTaskWhenDone that will never receive a done signal)
    mHandler = 0;

    Q_EMIT done(mTask); // FIXME this is more like a callback; queued connections will not work
  }
}
