
#include "SignalTask.hh"

SignalTask::~SignalTask()
{
  delete mWrapped;
}

void SignalTask::notifyDone()
{
  mWrapped->notifyDone();
  Q_EMIT signalDone();
}

void SignalTask::notifyError(QString message)
{
  mWrapped->notifyError(message);
  Q_EMIT signalError(message);
}
