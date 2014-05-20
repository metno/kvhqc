
#include "ModelBuffer.hh"

#include "SyncRequest.hh"

#include <boost/make_shared.hpp>

#define MILOGGER_CATEGORY "kvhqc.ModelBuffer"
#include "ObsLogging.hh"

ModelBuffer::ModelBuffer(ModelAccess_p ma)
  : mMA(ma)
{
  METLIBS_LOG_SCOPE();
}

ModelBuffer::~ModelBuffer()
{
  METLIBS_LOG_SCOPE();
  if (mRequest)
    dropRequest();
}

ModelData_p ModelBuffer::get(const SensorTime& st)
{
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
  METLIBS_LOG_SCOPE();
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

void ModelBuffer::onRequestCompleted(bool failed)
{
  METLIBS_LOG_SCOPE(LOGVAL(failed));
  dropRequest();
}

ModelRequest_p ModelBuffer::makeRequest(const SensorTime_v& sts)
{
  METLIBS_LOG_SCOPE(LOGVAL(sts.size()));
  ModelRequest_p request = boost::make_shared<ModelRequest>(sts);
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
  connect(mRequest.get(), SIGNAL(completed(bool)),
      this, SLOT(onRequestCompleted(bool)));
  mMA->postRequest(mRequest);
}

void ModelBuffer::dropRequest()
{
  METLIBS_LOG_SCOPE();
  disconnect(mRequest.get(), SIGNAL(completed(bool)),
      this, SLOT(onRequestCompleted(bool)));
  dropRequest(mRequest);
  mRequest = ModelRequest_p();
  if (not mPending.empty())
    postRequest();
}
