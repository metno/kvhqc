/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2014-2018 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: kvalobs-dev@met.no

  This file is part of HQC

  HQC is free software; you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  HQC is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General Public License
  along with HQC; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/


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
  if (it != mCache.end() and eq_ModelSensorTime()((*it)->sensorTime(), st)) {
    METLIBS_LOG_DEBUG("found" << LOGVAL((*it)->sensorTime()));
    return *it;
  }

  METLIBS_LOG_DEBUG(LOGVAL(st) << " not in cache");
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
  connect(request.get(), &ModelRequest::data, this, &ModelBuffer::onRequestData);
  connect(request.get(), &ModelRequest::requestCompleted, this, &ModelBuffer::onRequestCompleted);
  return request;
}

void ModelBuffer::postRequest()
{
  METLIBS_LOG_SCOPE();
  mRequest = makeRequest(mPending);
  mPending.clear();
  mMA->postRequest(mRequest);
}

void ModelBuffer::dropRequest()
{
  METLIBS_LOG_SCOPE();
  disconnect(mRequest.get(), &ModelRequest::data, this, &ModelBuffer::onRequestData);
  disconnect(mRequest.get(), &ModelRequest::requestCompleted, this, &ModelBuffer::onRequestCompleted);
  mMA->dropRequest(mRequest);
  mRequest = ModelRequest_p();
  if (!mPending.empty())
    postRequest();
}
