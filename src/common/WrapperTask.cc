
#include "WrapperTask.hh"

#define MILOGGER_CATEGORY "kvhqc.WrapperTask"
#include "common/ObsLogging.hh"

LOG_CONSTRUCT_COUNTER;

WrapperTask::WrapperTask(QueryTask* wrapped)
  : SignalTask(wrapped->priority())
  , mWrapped(wrapped)
{
  METLIBS_LOG_SCOPE("wrapped=" << mWrapped);
  LOG_CONSTRUCT();
}

WrapperTask::~WrapperTask()
{
  METLIBS_LOG_SCOPE("wrapped=" << mWrapped);
  LOG_DESTRUCT();
  delete mWrapped;
}

QString WrapperTask::querySql(QString dbversion) const
{
  return mWrapped->querySql(dbversion);
}

void WrapperTask::notifyRow(const ResultRow& row)
{
  mWrapped->notifyRow(row);
}

void WrapperTask::notifyStatus(int status)
{
  mWrapped->notifyStatus(status);
  SignalTask::notifyStatus(status);
}

void WrapperTask::notifyError(QString message)
{
  mWrapped->notifyError(message);
  SignalTask::notifyError(message);
}

void WrapperTask::notifyDone()
{
  METLIBS_LOG_SCOPE("this=" << this << " wrapped=" << mWrapped);
  mWrapped->notifyDone();
  SignalTask::notifyDone();
}
