
#include "ObsPgmRequest.hh"

#include "KvMetaDataBuffer.hh"
#include "ObsPgmQueryTask.hh"
#include "QueryTaskHandler.hh"
#include "QueryTaskHelper.hh"

#define MILOGGER_CATEGORY "kvhqc.ObsPgmRequest"
#include "util/HqcLogging.hh"

LOG_CONSTRUCT_COUNTER;

ObsPgmRequest::ObsPgmRequest(const hqc::int_s& stationIds)
{
  METLIBS_LOG_SCOPE();
  LOG_CONSTRUCT();
  init(stationIds);
}

ObsPgmRequest::ObsPgmRequest(int stationId)
{
  METLIBS_LOG_SCOPE();
  LOG_CONSTRUCT();
  hqc::int_s stationIds;
  stationIds.insert(stationId);
  init(stationIds);
}

void ObsPgmRequest::init(const hqc::int_s& stationIds)
{
  METLIBS_LOG_SCOPE(LOGVAL(stationIds.size()));
  hqc::int_s request;
  for (hqc::int_s::const_iterator itS = stationIds.begin(); itS != stationIds.end(); ++itS) {
    const hqc::kvObsPgm_v& op = KvMetaDataBuffer::instance()->findObsPgm(*itS);
    if (op.empty()) {
      request.insert(*itS);
    } else {
      mObsPgms[*itS] = op;
    }
  }
  METLIBS_LOG_DEBUG(LOGVAL(request.size()));
  if (request.empty()) {
    mTaskHelper = 0;
  } else {
    ObsPgmQueryTask* ot = new ObsPgmQueryTask(request, QueryTask::PRIORITY_AUTOMATIC);
    mTaskHelper = new QueryTaskHelper(ot);
    connect(mTaskHelper, SIGNAL(done(QueryTask*)), this, SLOT(onTaskDone(QueryTask*)));
  }
}

ObsPgmRequest::~ObsPgmRequest()
{
  METLIBS_LOG_SCOPE();
  LOG_DESTRUCT();
  delete mTaskHelper;
}

void ObsPgmRequest::post()
{
  METLIBS_LOG_SCOPE();
  if (mTaskHelper) {
    mTaskHelper->post(KvMetaDataBuffer::instance()->handler());
  } else {
    Q_EMIT complete();
  }
}

void ObsPgmRequest::sync()
{
  METLIBS_LOG_SCOPE();
  if (mTaskHelper) {
    mTaskHelper->post(KvMetaDataBuffer::instance()->handler(), true);
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
  KvMetaDataBuffer::instance()->putObsPgm(op);
  for (hqc::kvObsPgm_v::const_iterator it = op.begin(); it != op.end(); ++it)
    mObsPgms[it->stationID()].push_back(*it);
}

void ObsPgmRequest::onTaskDone(QueryTask* task)
{
  METLIBS_LOG_SCOPE();
  put(static_cast<ObsPgmQueryTask*>(task)->obsPgms());
  Q_EMIT complete();
}

// static
const hqc::kvObsPgm_v ObsPgmRequest::sEmpty;
