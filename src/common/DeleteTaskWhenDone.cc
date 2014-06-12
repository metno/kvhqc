
#include "DeleteTaskWhenDone.hh"

#include "SignalTask.hh"

DeleteTaskWhenDone::DeleteTaskWhenDone(SignalTask* t)
  : mTask(t)
{
  connect(mTask, SIGNAL(done()), this, SLOT(onQueryDone()));
}

void DeleteTaskWhenDone::onQueryDone()
{
  mTask->deleteLater();
  deleteLater();
}
