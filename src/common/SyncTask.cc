
#include "SyncTask.hh"

#include "QueryTask.hh"
#include "util/Synchronizer.hh"

QueryTask* syncTask(QueryTask* task, QueryTaskHandler* handler)
{
  Synchronizer sync;
  QObject::connect(task, SIGNAL(taskDone(const QString&)), &sync, SLOT(taskDone()));

  handler->postTask(task);
  sync.waitForSignal();

  return task;
}
