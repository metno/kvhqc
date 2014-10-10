
#include "QueryTask.hh"

QueryTask::QueryTask(size_t priority)
  : mPriority(priority)
{
}

QueryTask::~QueryTask()
{
}

void QueryTask::notifyDone(const QString& withError)
{
  Q_EMIT taskDone(withError);
}
