
#include "QueryTaskAccess.hh"

#include "DataQueryTask.hh"
#include "DistributeUpdates.hh"
#include "ObsAccept.hh"

#define MILOGGER_CATEGORY "kvhqc.QueryTaskAccess"
#include "common/ObsLogging.hh"

// ========================================================================

QueryTaskAccess::QueryTaskAccess(QueryTaskHandler_p handler)
  : mHandler(handler)
{
}

// ------------------------------------------------------------------------

QueryTaskAccess::~QueryTaskAccess()
{
}

// ------------------------------------------------------------------------

QueryTask* QueryTaskAccess::taskForRequest(ObsRequest_p request)
{
  DataQueryTask* task = new DataQueryTask(request, 10);
  connect(task, SIGNAL(newData(ObsRequest_p, const ObsData_pv&)),
      this, SLOT(onNewData(ObsRequest_p, const ObsData_pv&)));
  request->setTag(task);
  return task;
}

// ------------------------------------------------------------------------

void QueryTaskAccess::postRequest(ObsRequest_p request)
{
  METLIBS_LOG_SCOPE();
  mRequests.push_back(request);
  mHandler->postTask(taskForRequest(request));
}

// ------------------------------------------------------------------------

void QueryTaskAccess::dropRequest(ObsRequest_p request)
{
  METLIBS_LOG_SCOPE();

  // TODO what to do with requests that are processed in bg thread?
  ObsRequest_pv::iterator it = std::find(mRequests.begin(), mRequests.end(), request);
  if (it == mRequests.end()) {
    METLIBS_LOG_ERROR("dropping unknown request");
    return;
  }

  DataQueryTask* task = static_cast<DataQueryTask*>(request->tag());
  disconnect(task, SIGNAL(newData(ObsRequest_p, const ObsData_pv&)),
      this, SLOT(onNewData(ObsRequest_p, const ObsData_pv&)));
  mHandler->dropTask(task);
  delete task;
  request->setTag(0);

  mRequests.erase(it);
}

// ------------------------------------------------------------------------

void QueryTaskAccess::onNewData(ObsRequest_p request, const ObsData_pv& data)
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

void QueryTaskAccess::distributeUpdates(const ObsData_pv& updated, const ObsData_pv& inserted, const SensorTime_v& dropped)
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
