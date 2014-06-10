
#include "SignalTask.hh"

SignalTask::SignalTask(size_t priority)
  : QueryTask(priority)
{
}

void SignalTask::notifyStatus(int s)
{
  Q_EMIT status(s);
}

void SignalTask::notifyError(QString m)
{
  Q_EMIT error(m);
}

void SignalTask::notifyDone()
{
  Q_EMIT done();
}

// ########################################################################

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
