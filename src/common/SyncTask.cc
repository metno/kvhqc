
#include "SyncTask.hh"

#include "WrapperTask.hh"
#include "util/Synchronizer.hh"

#include <boost/make_shared.hpp>

QueryTask* syncTask(QueryTask* task, QueryTaskHandler* handler)
{
  SignalTask* stask = dynamic_cast<SignalTask*>(task);
  if (not stask)
    stask = new WrapperTask(task);

  Synchronizer sync;
  QObject::connect(stask, SIGNAL(done()), &sync, SLOT(taskDone()));

  handler->postTask(stask);
  sync.waitForSignal();

  return stask;
}
