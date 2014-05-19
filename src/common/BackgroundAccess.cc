
#include "BackgroundAccess.hh"
#include "BackgroundAccessPrivate.hh"

#include "DataQueryTask.hh"
#include "DistributeUpdates.hh"
#include "ObsAccept.hh"

#define MILOGGER_CATEGORY "kvhqc.BackgroundAccess"
#include "common/ObsLogging.hh"

// ========================================================================

BackgroundHandler::~BackgroundHandler()
{
}

// ========================================================================

// based on Qt4 blocking fortune client example

BackgroundThread::BackgroundThread(BackgroundHandler_p handler)
  : mHandler(handler)
  , mDone(false)
{
  METLIBS_LOG_SCOPE();
}

BackgroundThread::~BackgroundThread()
{
  METLIBS_LOG_SCOPE();
  mMutex.lock();
  mDone = true;
  mCondition.wakeOne();
  mMutex.unlock();
  wait();
}

void BackgroundThread::enqueueTask(QueryTask* task)
{
  METLIBS_LOG_SCOPE();

  QMutexLocker locker(&mMutex);
  mQueue.push(task);
  
  if (!isRunning())
    start();
  else
    mCondition.wakeOne();
}

void BackgroundThread::unqueueTask(QueryTask* task)
{
  METLIBS_LOG_SCOPE();

  QMutexLocker locker(&mMutex);
  mQueue.drop(task);
}

void BackgroundThread::run()
{
  METLIBS_LOG_SCOPE();
  mHandler->initialize();
  while (!mDone) {
    QueryTask* task = 0;

    mMutex.lock();
    if (mQueue.empty())
      mCondition.wait(&mMutex);
    if (not mQueue.empty()) {
      task = mQueue.top();
      mQueue.pop();
    }
    mMutex.unlock();

    if (task) {
      mHandler->queryTask(task);
      delete task;
    }
  }
  mHandler->finalize();
}

// ========================================================================

BackgroundAccess::BackgroundAccess(BackgroundHandler_p handler, bool useThread)
  : mHandler(handler)
  , mThread(0)
{
  METLIBS_LOG_SCOPE();
  if (useThread)
    mThread = new BackgroundThread(mHandler);
}

// ------------------------------------------------------------------------

BackgroundAccess::~BackgroundAccess()
{
  METLIBS_LOG_SCOPE();
  delete mThread;
}

// ------------------------------------------------------------------------

QueryTask* BackgroundAccess::taskForRequest(ObsRequest_p request)
{
  DataQueryTask* task = new DataQueryTask(request, 10);
  connect(task, SIGNAL(newData(ObsRequest_p, const ObsData_pv&)),
      this, SLOT(onNewData(ObsRequest_p, const ObsData_pv&)));
  return task;
}

// ------------------------------------------------------------------------

void BackgroundAccess::postRequest(ObsRequest_p request)
{
  METLIBS_LOG_SCOPE();
  mRequests.push_back(request);
  QueryTask* task = taskForRequest(request);
  if (mThread) {
    request->setTag(task);
    mThread->enqueueTask(task);
  } else {
    mHandler->queryTask(task);
    delete task;
  }
}

// ------------------------------------------------------------------------

void BackgroundAccess::dropRequest(ObsRequest_p request)
{
  METLIBS_LOG_SCOPE();

  // TODO what to do with requests that are processed in bg thread?
  ObsRequest_pv::iterator it = std::find(mRequests.begin(), mRequests.end(), request);
  if (it == mRequests.end()) {
    METLIBS_LOG_ERROR("dropping unknown request");
    return;
  }

  mRequests.erase(it);
  if (mThread) {
    mThread->unqueueTask(static_cast<QueryTask*>(request->tag()));
    request->setTag(0);
  }
}

// ------------------------------------------------------------------------

void BackgroundAccess::onNewData(ObsRequest_p request, const ObsData_pv& data)
{
  METLIBS_LOG_SCOPE();

  ObsRequest_pv::iterator it = std::find(mRequests.begin(), mRequests.end(), request);
  if (it != mRequests.end()) {
    METLIBS_LOG_DEBUG(LOGVAL(request->sensors()) << LOGVAL(request->timeSpan()));
    if (not data.empty())
      request->newData(data); // FIXME this is not exception safe
    else
      request->completed(false);
  } else {
    METLIBS_LOG_DEBUG("request has been dropped, do nothing");
  }
}

// ------------------------------------------------------------------------

void BackgroundAccess::distributeUpdates(const ObsData_pv& updated, const ObsData_pv& inserted, const SensorTime_v& dropped)
{
  METLIBS_LOG_SCOPE();
  DistributeRequestUpdates<ObsRequest_pv> du(mRequests);

  for (ObsData_pv::const_iterator itD = updated.begin(); itD != updated.end(); ++itD)
    du.updateData(*itD);

  for (ObsData_pv::const_iterator itI = inserted.begin(); itI != inserted.end(); ++itI)
    du.newData(*itI);

  for (SensorTime_v::const_iterator itD = dropped.begin(); itD != dropped.end(); ++itD)
    du.dropData(*itD);

  du.send();
}
