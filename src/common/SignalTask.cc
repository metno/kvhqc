
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
