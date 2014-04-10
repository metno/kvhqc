
#include "BackgroundAccess.hh"
#include "BackgroundAccessPrivate.hh"

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
  connect(mHandler.get(), SIGNAL(newData(ObsRequest_p, const ObsData_pv&)),
      this, SIGNAL(newData(ObsRequest_p, const ObsData_pv&)));
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

void BackgroundThread::enqueueRequest(ObsRequest_p request)
{
  METLIBS_LOG_SCOPE();
  QMutexLocker locker(&mMutex);
  mQueue.push(QueuedQuery(1, request));
  
  if (!isRunning())
    start();
  else
    mCondition.wakeOne();
}

void BackgroundThread::run()
{
  METLIBS_LOG_SCOPE();
  mHandler->initialize();
  while (!mDone) {
    ObsRequest_p request;

    mMutex.lock();
    if (mQueue.empty())
      mCondition.wait(&mMutex);
    if (not mQueue.empty()) {
      request = mQueue.top().request;
      mQueue.pop();
    }
    mMutex.unlock();

    if (request)
      mHandler->queryData(request);
  }
  mHandler->finalize();
}

// ========================================================================

BackgroundAccess::BackgroundAccess(BackgroundHandler_p handler, bool useThread)
  : mHandler(handler)
  , mThread(0)
{
  METLIBS_LOG_SCOPE();
  if (useThread) {
    mThread = new BackgroundThread(mHandler);
    connect(mThread, SIGNAL(newData(ObsRequest_p, const ObsData_pv&)),
        this, SLOT(onNewData(ObsRequest_p, const ObsData_pv&)));
  } else {
    connect(mHandler.get(), SIGNAL(newData(ObsRequest_p, const ObsData_pv&)),
        this, SLOT(onNewData(ObsRequest_p, const ObsData_pv&)));
  }
}

// ------------------------------------------------------------------------

BackgroundAccess::~BackgroundAccess()
{
  METLIBS_LOG_SCOPE();
  delete mThread;
}

// ------------------------------------------------------------------------

void BackgroundAccess::postRequest(ObsRequest_p request)
{
  METLIBS_LOG_SCOPE();
  mRequests.push_back(request);
  if (mThread)
    mThread->enqueueRequest(request);
  else
    mHandler->queryData(request);
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
  //if (mThread) // TODO
  //  mThread->unqueueRequest(request);
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

static bool acceptObs(ObsRequest_p request, ObsData_p obs)
{
  METLIBS_LOG_SCOPE(LOGVAL(request->sensors()) << LOGVAL(request->timeSpan()) << LOGVAL(obs->sensorTime()));
  const SensorTime& st = obs->sensorTime();
  if (not request->timeSpan().contains(st.time))
    return false;

  if (not request->sensors().count(st.sensor))
    return false;

  ObsFilter_p filter = request->filter();
  if (filter and not filter->accept(obs, false))
    return false;

  METLIBS_LOG_DEBUG("accepted");
  return true;
}

void BackgroundAccess::distributeUpdates(const ObsData_pv& updated, const ObsData_pv& inserted)
{
  METLIBS_LOG_SCOPE();
  typedef std::map<ObsRequest_p, ObsData_pv> r_obs_t;
  r_obs_t r_update, r_new;
  for (ObsRequest_pv::iterator itR = mRequests.begin(); itR != mRequests.end(); ++itR) {
    ObsRequest_p request = *itR;

    for (ObsData_pv::const_iterator itD = updated.begin(); itD != updated.end(); ++itD) {
      if (acceptObs(request, *itD))
        r_update[request].push_back(*itD);
    }

    for (ObsData_pv::const_iterator itI = inserted.begin(); itI != inserted.end(); ++itI) {
      if (acceptObs(request, *itI))
        r_new[request].push_back(*itI);
    }
  }

  for (r_obs_t::const_iterator it=r_update.begin(); it != r_update.end(); ++it) {
    ObsRequest_p request = it->first;
    request->updateData(it->second);
  }

  for (r_obs_t::const_iterator it=r_new.begin(); it != r_new.end(); ++it) {
    ObsRequest_p request = it->first;
    request->newData(it->second);
  }
}
