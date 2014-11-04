
#include "QueryTaskAccess.hh"

#include "DataQueryTask.hh"
#include "DeleteTaskWhenDone.hh"
#include "DistributeUpdates.hh"
#include "ObsAccept.hh"
#include "QueryTaskHelper.hh"

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

QueryTaskHelper* QueryTaskAccess::taskForRequest(ObsRequest_p request)
{
  DataQueryTask* task = new DataQueryTask(request, QueryTask::PRIORITY_INTERACTIVE);
  connect(task, SIGNAL(newData(ObsRequest_p, const ObsData_pv&)),
      this, SLOT(onNewData(ObsRequest_p, const ObsData_pv&)));
  connect(task, SIGNAL(queryDone(ObsRequest_p, const QString&)),
      this, SLOT(onDone(ObsRequest_p, const QString&)));

  QueryTaskHelper* helper = new QueryTaskHelper(task);
  request->setTag(helper);
  return helper;
}

// ------------------------------------------------------------------------

void QueryTaskAccess::postRequest(ObsRequest_p request, bool synchronized)
{
  METLIBS_LOG_SCOPE();
  mRequests.push_back(request);
  taskForRequest(request)->post(mHandler, synchronized);
}

// ------------------------------------------------------------------------

namespace {
const DataQueryTask* unwrapTask(QueryTaskHelper* helper)
{
  return static_cast<const DataQueryTask*>(helper->task());
}
}

// ------------------------------------------------------------------------

void QueryTaskAccess::dropRequest(ObsRequest_p request)
{
  METLIBS_LOG_SCOPE();

  // TODO what to do with requests that are processed in bg thread?
  ObsRequest_pv::iterator it = std::find(mRequests.begin(), mRequests.end(), request);
  if (it == mRequests.end()) {
    HQC_LOG_ERROR("dropping unknown request");
    return;
  }

  QueryTaskHelper* helper = static_cast<QueryTaskHelper*>(request->tag());
  const DataQueryTask* task = unwrapTask(helper);
  disconnect(task, SIGNAL(newData(ObsRequest_p, const ObsData_pv&)),
      this, SLOT(onNewData(ObsRequest_p, const ObsData_pv&)));
  disconnect(task, SIGNAL(queryDone(ObsRequest_p, const QString&)),
      this, SLOT(onDone(ObsRequest_p, const QString&)));
  delete helper;
  request->setTag(0);

  mRequests.erase(it);
}

// ------------------------------------------------------------------------

void QueryTaskAccess::onNewData(ObsRequest_p request, const ObsData_pv& data)
{
  METLIBS_LOG_SCOPE();

  if (isKnownRequest(request)) {
    METLIBS_LOG_DEBUG(LOGVAL(request->sensors()) << LOGVAL(request->timeSpan()));
    request->newData(data); // FIXME this is not exception safe
  } else {
    METLIBS_LOG_DEBUG("request has been dropped, do nothing");
  }
}

// ------------------------------------------------------------------------

void QueryTaskAccess::onDone(ObsRequest_p request, const QString& withError)
{
  METLIBS_LOG_SCOPE();
  if (isKnownRequest(request)) {
    METLIBS_LOG_DEBUG(LOGVAL(request->sensors()) << LOGVAL(request->timeSpan()));
    request->completed(withError);
  } else {
    METLIBS_LOG_DEBUG("request has been dropped, do nothing");
  }
}

bool QueryTaskAccess::isKnownRequest(ObsRequest_p request) const
{
  return (std::find(mRequests.begin(), mRequests.end(), request) != mRequests.end());
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
