
#include "ModelBuffer.hh"

#include "ModelAccess.hh"
#include "ModelRequest.hh"
#include "SyncRequest.hh"

#define MILOGGER_CATEGORY "kvhqc.ModelBuffer"
#include "ObsLogging.hh"

LOG_CONSTRUCT_COUNTER;

ModelBuffer::ModelBuffer(ModelAccess_p ma)
  : mMA(ma)
{
  METLIBS_LOG_SCOPE();
  LOG_CONSTRUCT();
}

ModelBuffer::~ModelBuffer()
{
  METLIBS_LOG_SCOPE();
  LOG_DESTRUCT();
  if (mRequest) {
    mPending.clear(); // avoid posting a new request
    dropRequest();
  }
}

ModelData_p ModelBuffer::get(const SensorTime& st)
{
  METLIBS_LOG_SCOPE(st);
  ModelData_p c = cached(st);
  if (not c) {
    mPending.push_back(st);
    if (not mRequest)
      postRequest();
  }
  return c;
}

ModelData_p ModelBuffer::getSync(const SensorTime& st)
{
  METLIBS_LOG_SCOPE(st);
  ModelData_p c = cached(st);
  if (not c) {
    ModelRequest_p sRequest = makeRequest(SensorTime_v(1, st));
    syncRequest(sRequest, mMA);
    mMA->dropRequest(sRequest);
    c = cached(st);
  }
  return c;
}

void ModelBuffer::clear()
{
  METLIBS_LOG_SCOPE();
  mCache.clear();
  mPending.clear();
}

ModelData_p ModelBuffer::cached(const SensorTime& st)
{
  METLIBS_LOG_SCOPE(st);
  ModelData_ps::const_iterator it = std::lower_bound(mCache.begin(), mCache.end(), st, lt_ModelData_p());
  if (it != mCache.end() and eq_ModelSensorTime()((*it)->sensorTime(), st))
    return *it;

  METLIBS_LOG_DEBUG(LOGVAL(st) << " not in cache");
  if (it != mCache.end())
    METLIBS_LOG_DEBUG("found" << LOGVAL((*it)->sensorTime()));
  return ModelData_p();
}

void ModelBuffer::onRequestData(const ModelData_pv& mdata)
{
  METLIBS_LOG_SCOPE(LOGVAL(mdata.size()));
  mCache.insert(mdata.begin(), mdata.end());
  Q_EMIT received(mdata);
}

void ModelBuffer::onRequestCompleted(const QString& withError)
{
  METLIBS_LOG_SCOPE(LOGVAL(withError));
  dropRequest();
}

ModelRequest_p ModelBuffer::makeRequest(const SensorTime_v& sts)
{
  METLIBS_LOG_SCOPE(LOGVAL(sts.size()));
  ModelRequest_p request = std::make_shared<ModelRequest>(sts);
  connect(request.get(), SIGNAL(data(const ModelData_pv&)),
      this, SLOT(onRequestData(const ModelData_pv&)));
  return request;
}

void ModelBuffer::dropRequest(ModelRequest_p request)
{
  METLIBS_LOG_SCOPE();
  disconnect(request.get(), SIGNAL(data(const ModelData_pv&)),
      this, SLOT(onRequestData(const ModelData_pv&)));
  METLIBS_LOG_DEBUG("dropping from ModelAccess");
  mMA->dropRequest(request);
}

void ModelBuffer::postRequest()
{
  METLIBS_LOG_SCOPE();
  mRequest = makeRequest(mPending);
  mPending.clear();
  connect(mRequest.get(), SIGNAL(requestCompleted(const QString&)),
      this, SLOT(onRequestCompleted(const QString&)));
  mMA->postRequest(mRequest);
}

void ModelBuffer::dropRequest()
{
  METLIBS_LOG_SCOPE();
  disconnect(mRequest.get(), SIGNAL(requestCompleted(const QString&)),
      this, SLOT(onRequestCompleted(const QString&)));
  dropRequest(mRequest);
  mRequest = ModelRequest_p();
  if (not mPending.empty())
    postRequest();
}
