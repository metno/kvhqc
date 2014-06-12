
#include "QueryTaskHelper.hh"

#include "DeleteTaskWhenDone.hh"
#include "QueryTaskHandler.hh"
#include "SignalTask.hh"

QueryTaskHelper::QueryTaskHelper(SignalTask* task)
  : mHandler(0)
  , mTask(task)
{
}

QueryTaskHelper::~QueryTaskHelper()
{
  if (not drop()) {
    // posted but not yet done; cannot delete task before done was
    // emitted (as QueryTaskHandler still makes calls to it), so we
    // create an object to wait for the done signal
    new DeleteTaskWhenDone(mTask);
  }
}

void QueryTaskHelper::post(QueryTaskHandler* handler)
{
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
  if (not mTask)
    return true;
  QueryTaskHandler* handler = mHandler;
  mHandler = 0;
  if (not handler or handler->dropTask(mTask)) {
    delete mTask;
    mTask = 0;
    return true;
  } else {
    disconnect(mTask, SIGNAL(done()), this, SLOT(onQueryDone()));
    return false;
  }
}

void QueryTaskHelper::onQueryDone()
{
  if (mHandler and mTask) {
    Q_EMIT done(mTask); // FIXME this is more like a callback; queued connections will not work
    mHandler = 0;
  }
}
