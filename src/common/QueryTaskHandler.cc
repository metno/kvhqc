
#include "QueryTaskHandler.hh"
#include "QueryTaskHandlerPrivate.hh"

#define MILOGGER_CATEGORY "kvhqc.QueryTaskHandler"
#include "common/ObsLogging.hh"

// ========================================================================

QueryTaskRunner::~QueryTaskRunner()
{
}

// ========================================================================

// based on Qt4 blocking fortune client example

QueryTaskThread::QueryTaskThread(QueryTaskRunner_p runner)
  : mRunner(runner)
  , mDone(false)
{
  METLIBS_LOG_SCOPE();
}

QueryTaskThread::~QueryTaskThread()
{
  METLIBS_LOG_SCOPE();
  mMutex.lock();
  mDone = true;
  mCondition.wakeOne();
  mMutex.unlock();
  wait();
}

void QueryTaskThread::enqueueTask(QueryTask* task)
{
  METLIBS_LOG_SCOPE();

  QMutexLocker locker(&mMutex);
  mQueue.push(task);
  
  if (!isRunning())
    start();
  else
    mCondition.wakeOne();
}

bool QueryTaskThread::unqueueTask(QueryTask* task)
{
  METLIBS_LOG_SCOPE();

  QMutexLocker locker(&mMutex);
  return mQueue.drop(task);
}

void QueryTaskThread::run()
{
  METLIBS_LOG_SCOPE();
  mRunner->initialize();
  while (!mDone) {
    QueryTask* task = 0;

    {
      QMutexLocker locker(&mMutex);
      if (mQueue.empty())
        mCondition.wait(&mMutex);
      if (not mQueue.empty()) {
        task = mQueue.top();
        mQueue.pop();
      }
    }

    if (task) {
      mRunner->run(task);
      task->notifyDone();
    }
  }
  mRunner->finalize();
}

// ========================================================================

QueryTaskHandler::QueryTaskHandler(QueryTaskRunner_p runner, bool useThread)
  : mRunner(runner)
  , mThread(0)
{
  METLIBS_LOG_SCOPE();
  if (useThread) {
    mThread = new QueryTaskThread(mRunner);
  } else {
    mRunner->initialize();
  }
}

// ------------------------------------------------------------------------

QueryTaskHandler::~QueryTaskHandler()
{
  METLIBS_LOG_SCOPE();
  if (mThread) {
    delete mThread;
  } else {
    mRunner->finalize();
  }
}

// ------------------------------------------------------------------------

void QueryTaskHandler::postTask(QueryTask_x task)
{
  METLIBS_LOG_SCOPE();
  if (mThread)
    mThread->enqueueTask(task);
  else
    mRunner->run(task);
}

// ------------------------------------------------------------------------

bool QueryTaskHandler::dropTask(QueryTask_x task)
{
  METLIBS_LOG_SCOPE();
  
  if (mThread)
    return mThread->unqueueTask(task);
  else
    return false;
}
