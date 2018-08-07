
#include "KvalobsModelAccess.hh"

#include "DeleteTaskWhenDone.hh"
#include "ModelQueryTask.hh"
#include "ModelRequest.hh"
#include "QueryTaskHandler.hh"
#include "QueryTaskHelper.hh"

#define MILOGGER_CATEGORY "kvhqc.KvalobsModelAccess"
#include "common/ObsLogging.hh"

KvalobsModelAccess::KvalobsModelAccess(QueryTaskHandler_p handler)
  : mHandler(handler)
{
}

KvalobsModelAccess::~KvalobsModelAccess()
{
}

void KvalobsModelAccess::cleanCache()
{
  mCache.clear();
}

void KvalobsModelAccess::postRequest(ModelRequest_p request)
{
  METLIBS_LOG_SCOPE();
  mRequests.push_back(request);

  const SensorTime_v& requested = request->sensorTimes();
  SensorTime_v toQuery;
  ModelData_pv cached;
  METLIBS_LOG_DEBUG(LOGVAL(requested.size()));

  for (SensorTime_v::const_iterator it = requested.begin(); it != requested.end(); ++it) {
    METLIBS_LOG_DEBUG(LOGVAL(*it));
    ModelDataCache_t::iterator itC = mCache.find(*it);
    if (itC != mCache.end()) {
      if (itC->second)
        cached.push_back(itC->second);
    } else {
      toQuery.push_back(*it);
      // insert null pointer into cache to prevent repeated lookup for missing model values
      mCache.insert(std::make_pair(*it, ModelData_p()));
    }
  }

  METLIBS_LOG_DEBUG(LOGVAL(cached.size()) << LOGVAL(toQuery.size()));
  if (not cached.empty())
    request->notifyData(cached);

  if (not toQuery.empty()) {
    ModelQueryTask* task = new ModelQueryTask(toQuery, QueryTask::PRIORITY_AUTOMATIC);
    connect(task, &ModelQueryTask::data, request.get(), &ModelRequest::notifyData);
    connect(task, &ModelQueryTask::taskDone, request.get(), &ModelRequest::notifyDone);
    connect(task, &ModelQueryTask::data, this, &KvalobsModelAccess::modelData);

    QueryTaskHelper* helper = new QueryTaskHelper(task);
    request->setTag(helper);
    helper->post(mHandler);
  } else {
    request->notifyDone(QString());
  }
}

namespace {
const ModelQueryTask* unwrapTask(QueryTaskHelper* helper)
{
  return static_cast<const ModelQueryTask*>(helper->task());
}
}

void KvalobsModelAccess::dropRequest(ModelRequest_p request)
{
  METLIBS_LOG_SCOPE();
  ModelRequest_pv::iterator it = std::find(mRequests.begin(), mRequests.end(), request);
  if (it == mRequests.end()) {
    HQC_LOG_ERROR("dropping unknown model request");
    return;
  }

  if (QueryTaskHelper* helper = static_cast<QueryTaskHelper*>(request->tag())) {
    // request has no tag if it was fulfilled from the cache

    const ModelQueryTask* task = unwrapTask(helper);
    disconnect(task, &ModelQueryTask::data, request.get(), &ModelRequest::notifyData);
    disconnect(task, &ModelQueryTask::taskDone, request.get(), &ModelRequest::notifyDone);
    disconnect(task, &ModelQueryTask::data, this, &KvalobsModelAccess::modelData);
    delete helper;
    request->setTag(0);
  }

  mRequests.erase(it);
}

void KvalobsModelAccess::modelData(const ModelData_pv& mdata)
{
  METLIBS_LOG_SCOPE();
  for (ModelData_pv::const_iterator it = mdata.begin(); it != mdata.end(); ++it)
    mCache[(*it)->sensorTime()] = *it; // overwrite existing cache entry
}
