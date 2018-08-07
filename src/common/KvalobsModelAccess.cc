/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2013-2018 met.no

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

  for (const SensorTime& st : requested) {
    METLIBS_LOG_DEBUG(LOGVAL(st));
    ModelDataCache_t::iterator itC = mCache.find(st);
    if (itC != mCache.end()) {
      if (itC->second)
        cached.push_back(itC->second);
    } else {
      toQuery.push_back(st);
      // insert null pointer into cache to prevent repeated lookup for missing model values
      mCache.insert(std::make_pair(st, ModelData_p()));
    }
  }

  METLIBS_LOG_DEBUG(LOGVAL(cached.size()) << LOGVAL(toQuery.size()));
  if (!cached.empty())
    request->notifyData(cached);

  if (!toQuery.empty()) {
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
  for (ModelData_p m : mdata)
    mCache[m->sensorTime()] = m; // overwrite existing cache entry
}
