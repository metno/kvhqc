
#include "KvalobsModelAccess.hh"

#include "DeleteTaskWhenDone.hh"
#include "HqcApplication.hh"
#include "ModelQueryTask.hh"
#include "QueryTaskHandler.hh"
#include "WrapperTask.hh"

#define MILOGGER_CATEGORY "kvhqc.KvalobsModelAccess"
#include "common/ObsLogging.hh"

KvalobsModelAccess::KvalobsModelAccess()
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
  if (not cached.empty()) {
    // FIXME STARTED might come again when there are data to fetch
    // from the database
    request->notifyStatus(QueryTask::STARTED);

    request->notifyData(cached);
  }

  if (not toQuery.empty()) {
    ModelQueryTask* task = new ModelQueryTask(toQuery, QueryTask::PRIORITY_AUTOMATIC);
    connect(task, SIGNAL(data(const ModelData_pv&)),
        request.get(), SLOT(notifyData(const ModelData_pv&)));
    connect(task, SIGNAL(queryStatus(int)),
        request.get(), SLOT(notifyStatus(int)));
    connect(task, SIGNAL(data(const ModelData_pv&)),
        this, SLOT(modelData(const ModelData_pv&)));

    request->setTag(task);
    hqcApp->kvalobsHandler()->postTask(task);
  } else {
    request->notifyStatus(QueryTask::COMPLETE);
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

  ModelQueryTask* task = static_cast<ModelQueryTask*>(request->tag());
  if (task) {
    // request has no tag if it was fulfilled from the cache

    request->setTag(0);
    disconnect(task, SIGNAL(data(const ModelData_pv&)),
        request.get(), SIGNAL(notifyData(const ModelData_pv&)));
    disconnect(task, SIGNAL(queryStatus(int)),
        request.get(), SLOT(notifyStatus(int)));
    disconnect(task, SIGNAL(data(const ModelData_pv&)),
        this, SLOT(modelData(const ModelData_pv&)));
    if (hqcApp->kvalobsHandler()->dropTask(task))
      delete task;
    else
      new DeleteTaskWhenDone(new WrapperTask(task));
  }

  mRequests.erase(it);
}

void KvalobsModelAccess::modelData(const ModelData_pv& mdata)
{
  METLIBS_LOG_SCOPE();
  for (ModelData_pv::const_iterator it = mdata.begin(); it != mdata.end(); ++it)
    mCache[(*it)->sensorTime()] = *it; // overwrite existing cache entry
}
