
#include "ObsPgmRequest.hh"

#include "HqcApplication.hh"
#include "KvMetaDataBuffer.hh"
#include "ObsPgmQueryTask.hh"
#include "QueryTaskHandler.hh"
#include "QueryTaskHelper.hh"

#define MILOGGER_CATEGORY "kvhqc.ObsPgmRequest"
#include "util/HqcLogging.hh"

ObsPgmRequest::ObsPgmRequest(const std::set<int>& stationIds)
{
  hqc::int_s request;
  for (hqc::int_s::const_iterator itS = stationIds.begin(); itS != stationIds.end(); ++itS) {
    const hqc::kvObsPgm_v& op = KvMetaDataBuffer::instance()->findObsPgm(*itS);
    if (op.empty()) {
      request.insert(*itS);
    } else {
      put(op);
    }
  }
  if (request.empty()) {
    mTaskHelper = 0;
  } else {
    ObsPgmQueryTask* ot = new ObsPgmQueryTask(request, QueryTask::PRIORITY_AUTOMATIC);
    mTaskHelper = new QueryTaskHelper(ot);
    connect(mTaskHelper, SIGNAL(done(SignalTask*)), this, SLOT(onTaskDone(SignalTask*)));
  }
}

ObsPgmRequest::~ObsPgmRequest()
{
  delete mTaskHelper;
}

void ObsPgmRequest::post()
{
  if (mTaskHelper) {
    mTaskHelper->post(hqcApp->kvalobsHandler().get());
  } else {
    Q_EMIT complete();
  }
}

const hqc::kvObsPgm_v& ObsPgmRequest::get(int stationId) const
{
  const kvObsPgm_m::const_iterator it = mObsPgms.find(stationId);
  if (it != mObsPgms.end())
    return it->second;
  else
    return sEmpty;
}

void ObsPgmRequest::put(const hqc::kvObsPgm_v& op)
{
  for (hqc::kvObsPgm_v::const_iterator it = op.begin(); it != op.end(); ++it)
    mObsPgms[it->stationID()].push_back(*it);
}

void ObsPgmRequest::onTaskDone(SignalTask* task)
{
  put(static_cast<ObsPgmQueryTask*>(task)->obsPgms());
  Q_EMIT complete();
}

// static
const hqc::kvObsPgm_v ObsPgmRequest::sEmpty;
