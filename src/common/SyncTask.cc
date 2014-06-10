
#include "SyncTask.hh"

#include "SignalTask.hh"
#include "util/Synchronizer.hh"

#include <boost/make_shared.hpp>

QueryTask* syncTask(QueryTask* task, QueryTaskHandler_p handler)
{
  SignalTask* stask = dynamic_cast<SignalTask*>(task);
  if (not stask)
    stask = new WrapperTask(task);

  Synchronizer sync;
  QObject::connect(stask, SIGNAL(signalDone()), &sync, SLOT(taskDone()));
  QObject::connect(stask, SIGNAL(signalError(QString)), &sync, SLOT(taskDone()));

  handler->postTask(stask);
  sync.waitForSignal();

  QObject::disconnect(stask, SIGNAL(signalDone()), &sync, SLOT(taskDone()));
  QObject::disconnect(stask, SIGNAL(signalError(QString)), &sync, SLOT(taskDone()));

  return stask;
}
