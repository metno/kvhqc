
#include "QueryTask.hh"

#define MILOGGER_CATEGORY "kvhqc.QueryTask"
#include "util/HqcLogging.hh"

LOG_CONSTRUCT_COUNTER;

ResultRow::~ResultRow()
{
}

BasicSQLTask::BasicSQLTask()
  : mDone(false)
  , mDeleteWhenDone(false)
{
  METLIBS_LOG_SCOPE();
  LOG_CONSTRUCT();
}

BasicSQLTask::~BasicSQLTask()
{
  METLIBS_LOG_SCOPE();
  LOG_DESTRUCT();
}
  
void BasicSQLTask::deleteWhenDone()
{
  QMutexLocker locker(&mMutex);
  if (mDone)
    deleteLater();
  else
    mDeleteWhenDone = true;
}

void BasicSQLTask::notifyDone(const QString& withError)
{
  Q_EMIT taskDone(withError);

  QMutexLocker locker(&mMutex);
  mDone = true;
  if (mDeleteWhenDone)
    deleteLater();
}
