
#include "WrapperTask.hh"

WrapperTask::WrapperTask(QueryTask* wrapped)
  : SignalTask(wrapped->priority())
  , mWrapped(wrapped)
{
}

WrapperTask::~WrapperTask()
{
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
  mWrapped->notifyDone();
  SignalTask::notifyDone();
}
