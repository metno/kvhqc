
#include "QueryTask.hh"

QueryTask::QueryTask(size_t priority)
  : mPriority(priority)
{
}

QueryTask::~QueryTask()
{
}

void QueryTask::notifyError(QString)
{
}

void QueryTask::notifyDone()
{
}
