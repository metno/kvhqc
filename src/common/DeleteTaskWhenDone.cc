
#include "DeleteTaskWhenDone.hh"

#include "SignalTask.hh"

#define MILOGGER_CATEGORY "kvhqc.DeleteTaskWhenDone"
#include "util/HqcLogging.hh"

LOG_CONSTRUCT_COUNTER;

DeleteTaskWhenDone::DeleteTaskWhenDone(SignalTask* t)
  : mTask(t)
  , mDone(false) // FIXME this is actually an assumption
  , mDelete(false)
{
  METLIBS_LOG_SCOPE("task=" << mTask);
  LOG_CONSTRUCT();
  connect(mTask, SIGNAL(done()), this, SLOT(onQueryDone()));
}

DeleteTaskWhenDone::~DeleteTaskWhenDone()
{
  METLIBS_LOG_SCOPE();
  LOG_DESTRUCT();
}

void DeleteTaskWhenDone::doDelete()
{
  METLIBS_LOG_SCOPE();
  if (mTask) {
    delete mTask;//->deleteLater();
    mTask = 0;
  }
  delete this;//Later();
}

void DeleteTaskWhenDone::onQueryDone()
{
  METLIBS_LOG_SCOPE();
  mDone = true;
  if (mDelete)
    doDelete();
}

void DeleteTaskWhenDone::enableDelete()
{
  mDelete = true;
  if (mDone)
    doDelete();
}
