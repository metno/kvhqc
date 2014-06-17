
#include "DeleteTaskWhenDone.hh"

#include "SignalTask.hh"

#define MILOGGER_CATEGORY "kvhqc.DeleteTaskWhenDone"
#include "util/HqcLogging.hh"

LOG_CONSTRUCT_COUNTER;

DeleteTaskWhenDone::DeleteTaskWhenDone(SignalTask* t)
  : mTask(t)
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

void DeleteTaskWhenDone::onQueryDone()
{
  METLIBS_LOG_SCOPE();
  mTask->deleteLater();
  deleteLater();
}
