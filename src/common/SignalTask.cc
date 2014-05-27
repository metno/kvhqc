
#include "SignalTask.hh"

SignalTask::~SignalTask()
{
  delete mWrapped;
}

void SignalTask::notifyStatus(int status)
{
  mWrapped->notifyStatus(status);
  Q_EMIT signalStatus(status);
}

void SignalTask::notifyError(QString message)
{
  mWrapped->notifyError(message);
  Q_EMIT signalError(message);
}
