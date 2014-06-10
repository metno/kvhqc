
#include "KvMetaDataBuffer.hh"

#include "Functors.hh"
#include "HqcApplication.hh"
#include "ObsPgmQueryTask.hh"
#include "KvHelpers.hh"
#include "ParamQueryTask.hh"
#include "QueryTaskHandler.hh"
#include "StationQueryTask.hh"
#include "TypesQueryTask.hh"

#include <puTools/miStringBuilder.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QVariant>
#include <QtGui/QMessageBox>
#include <QtSql/QSqlQuery>

#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/range/adaptor/map.hpp>

#define MILOGGER_CATEGORY "kvhqc.KvMetaDataBuffer"
#include "common/ObsLogging.hh"

ObsPgmRequest::ObsPgmRequest(const std::set<int>& stationIds)
{
  int_s request;
  for (int_s::const_iterator itS = stationIds.begin(); itS != stationIds.end(); ++itS) {
    const kvObsPgm_v& op = KvMetaDataBuffer::instance()->findObsPgm(*itS);
    if (op.empty()) {
      request.insert(*itS);
    } else {
      put(op);
    }
  }
  if (request.empty()) {
    mTask = 0;
    mPosted = true;
  } else {
    mTask = new ObsPgmQueryTask(request, QueryTask::PRIORITY_AUTOMATIC);
    connect(mTask, SIGNAL(queryStatus(int)), this, SLOT(onTaskStatus(int)));
    mPosted = false;
  }
}

ObsPgmRequest::~ObsPgmRequest()
{
    delete mTask;
}

void ObsPgmRequest::post()
{
  if (mTask) {
    if (not mPosted)
      hqcApp->kvalobsHandler()->postTask(mTask);
  } else {
    Q_EMIT complete();
  }
}

const ObsPgmRequest::kvObsPgm_v& ObsPgmRequest::operator[](int stationId) const
{
  const kvObsPgm_m::const_iterator it = mObsPgms.find(stationId);
  if (it != mObsPgms.end())
    return it->second;
  else
    return sEmpty;
}

void ObsPgmRequest::put(const kvObsPgm_v& op)
{
  for (kvObsPgm_v::const_iterator it = op.begin(); it != op.end(); ++it)
    mObsPgms[it->stationID()].push_back(*it);
}

void ObsPgmRequest::onTaskStatus(int status)
{
  if (status == QueryTask::COMPLETE)
    put(mTask->obsPgms());
  if (status >= QueryTask::COMPLETE) {
    hqcApp->kvalobsHandler()->dropTask(mTask);
    delete mTask;
    mTask = 0;
    Q_EMIT complete();
  }
}

// static
const ObsPgmRequest::kvObsPgm_v ObsPgmRequest::sEmpty;

// ########################################################################

KvMetaDataBuffer* KvMetaDataBuffer::sInstance = 0;

KvMetaDataBuffer::KvMetaDataBuffer()
  : mHaveStations(false)
  , mTaskStations(0)
  , mHaveParams(false)
  , mTaskParams(0)
  , mHaveTypes(false)
  , mTaskTypes(0)
{
  assert(not sInstance);
  sInstance = this;
}

KvMetaDataBuffer::~KvMetaDataBuffer()
{
#if 0
  // FIXME this is called after eol of hqcApp->kvalobsHandler()
  dropStations();
  dropParams();
  dropTypes();
#endif
  sInstance = 0;
}

bool KvMetaDataBuffer::isKnownStation(int id)
{
  const kvStation_v& all = allStations();
  kvStation_v::const_iterator it = std::lower_bound(all.begin(), all.end(), id, Helpers::kvStation_lt_id());
  return (it != all.end() and it->stationID() == id);
}

const kvalobs::kvStation& KvMetaDataBuffer::findStation(int id)
{
  const kvStation_v& all = allStations();
  kvStation_v::const_iterator it = std::lower_bound(all.begin(), all.end(), id, Helpers::kvStation_lt_id());
  if (it != all.end() and it->stationID() == id)
    return *it;
  throw std::runtime_error(miutil::StringBuilder() << "station " << id << "' not found");
}

const std::vector<kvalobs::kvStation>& KvMetaDataBuffer::allStations()
{
  static kvStation_v empty;
  if (mTaskStations and mHaveStations)
    return mTaskStations->stations();
  return empty;
}

bool KvMetaDataBuffer::isKnownParam(int id)
{
  const kvParam_v& all = allParams();
  kvParam_v::const_iterator it = std::lower_bound(all.begin(), all.end(), id, Helpers::kvParam_lt_id());
  return (it != all.end() and it->paramID() == id);
}

const kvalobs::kvParam& KvMetaDataBuffer::findParam(int id)
{
  const kvParam_v& all = allParams();
  kvParam_v::const_iterator it = std::lower_bound(all.begin(), all.end(), id, Helpers::kvParam_lt_id());
  if (it != all.end() and it->paramID() == id)
    return *it;
  throw std::runtime_error(miutil::StringBuilder() << "param " << id << "' not found");
}

const std::vector<kvalobs::kvParam>& KvMetaDataBuffer::allParams()
{
  static kvParam_v empty;
  if (mTaskParams and mHaveParams)
    return mTaskParams->params();
  return empty;
}

std::string KvMetaDataBuffer::findParamName(int id)
{
  const kvParam_v& all = allParams();
  kvParam_v::const_iterator it = std::lower_bound(all.begin(), all.end(), id, Helpers::kvParam_lt_id());
  if (it != all.end() and it->paramID() == id)
    return it->name();
  
  return (boost::format("{%1$d}") % id).str();
}

bool KvMetaDataBuffer::isCodeParam(int paramid)
{
  return mCodeParams.find(paramid) != mCodeParams.end();
}

bool KvMetaDataBuffer::isDirectionInDegreesParam(int pid)
{
  const kvalobs::kvParam& param = findParam(pid);
  return (param.unit().find("grader") != std::string::npos
      and pid != kvalobs::PARAMID_MLON
      and pid != kvalobs::PARAMID_MLAT);
}

namespace /* anonymous */ {
const int NOPARAMMODEL     = 8;
const int modelParam[NOPARAMMODEL] =
{ 61, 81, 109, 110, 177, 178, 211, 262 };
} // anonymous namespace

bool KvMetaDataBuffer::isModelParam(int paramid)
{
  return std::binary_search(modelParam, boost::end(modelParam), paramid);
}

CachedParamLimits::ParamLimit KvMetaDataBuffer::checkPhysicalLimits(const SensorTime& st, float value)
{
  return mCachedParamLimits.check(st, value);
}

bool KvMetaDataBuffer::isKnownType(int id)
{
  const kvTypes_v& all = allTypes();
  kvTypes_v::const_iterator it = std::lower_bound(all.begin(), all.end(), id, Helpers::kvTypes_lt_id());
  return (it != all.end() and it->typeID() == id);
}

const kvalobs::kvTypes& KvMetaDataBuffer::findType(int id)
{
  const kvTypes_v& all = allTypes();
  kvTypes_v::const_iterator it = std::lower_bound(all.begin(), all.end(), id, Helpers::kvTypes_lt_id());
  if (it != all.end() and it->typeID() == id)
    return *it;
  throw std::runtime_error(miutil::StringBuilder() << "type " << id << "' not found");
}

const std::vector<kvalobs::kvTypes>& KvMetaDataBuffer::allTypes()
{
  static kvTypes_v empty;
  if (mTaskTypes and mHaveTypes)
    return mTaskTypes->types();
  return empty;
}

const KvMetaDataBuffer::kvObsPgm_v& KvMetaDataBuffer::findObsPgm(int stationid)
{
  static kvObsPgm_v empty;

  const kvObsPgm_m::iterator it = mObsPgms.find(stationid);
  if (it != mObsPgms.end())
    return it->second;

  return empty;
}

void KvMetaDataBuffer::putObsPgm(const kvObsPgm_v& op)
{
  for (kvObsPgm_v::const_iterator it = op.begin(); it != op.end(); ++it)
    mObsPgms[it->stationID()].push_back(*it);
}

void KvMetaDataBuffer::statusStations(int status)
{
  METLIBS_LOG_SCOPE(LOGVAL(status));
  if (status == QueryTask::COMPLETE) {
    mHaveStations = true;
  } else if (status > QueryTask::COMPLETE) {
    HQC_LOG_ERROR("error while retrieving station list");
    mHaveStations = true;
  }
  sendComplete();
}

void KvMetaDataBuffer::statusParams(int status)
{
  METLIBS_LOG_SCOPE(LOGVAL(status));
  if (status >= QueryTask::COMPLETE) {
    mHaveParams = true;

    mCodeParams.clear();
    BOOST_FOREACH(const kvalobs::kvParam& p, allParams()) {
      if (p.unit().find("kode") != std::string::npos) {
        mCodeParams.insert(p.paramID());
      }
    }
  }

  sendComplete();
}

void KvMetaDataBuffer::statusTypes(int status)
{
  METLIBS_LOG_SCOPE(LOGVAL(status));
  if (status >= QueryTask::COMPLETE)
    mHaveTypes = true;
  sendComplete();
}

void KvMetaDataBuffer::sendComplete()
{
  if (isComplete())
    Q_EMIT complete();
}

void KvMetaDataBuffer::fetchStations()
{
  METLIBS_LOG_SCOPE();
  dropStations();
  mTaskStations = new StationQueryTask(QueryTask::PRIORITY_AUTOMATIC);
  connect(mTaskStations, SIGNAL(queryStatus(int)), this, SLOT(statusStations(int)));
  hqcApp->kvalobsHandler()->postTask(mTaskStations);
}

void KvMetaDataBuffer::fetchParams()
{
  METLIBS_LOG_SCOPE();
  dropParams();
  mTaskParams = new ParamQueryTask(QueryTask::PRIORITY_AUTOMATIC);
  connect(mTaskParams, SIGNAL(queryStatus(int)), this, SLOT(statusParams(int)));
  hqcApp->kvalobsHandler()->postTask(mTaskParams);
}

void KvMetaDataBuffer::fetchTypes()
{
  METLIBS_LOG_SCOPE();
  dropTypes();
  mTaskTypes = new TypesQueryTask(QueryTask::PRIORITY_AUTOMATIC);
  connect(mTaskTypes, SIGNAL(queryStatus(int)), this, SLOT(statusTypes(int)));
  hqcApp->kvalobsHandler()->postTask(mTaskTypes);
}

void KvMetaDataBuffer::dropStations()
{
  METLIBS_LOG_SCOPE();
  if (mTaskStations) {
    hqcApp->kvalobsHandler()->dropTask(mTaskStations);
    delete mTaskStations;
    mTaskStations = 0;
  }
}

void KvMetaDataBuffer::dropParams()
{
  METLIBS_LOG_SCOPE();
  if (mTaskParams) {
    hqcApp->kvalobsHandler()->dropTask(mTaskParams);
    delete mTaskParams;
    mTaskParams = 0;
  }
}

void KvMetaDataBuffer::dropTypes()
{
  METLIBS_LOG_SCOPE();
  if (mTaskTypes) {
    hqcApp->kvalobsHandler()->dropTask(mTaskTypes);
    delete mTaskTypes;
    mTaskTypes = 0;
  }
}

void KvMetaDataBuffer::reload()
{
  mHaveStations = mHaveParams = mHaveTypes = false;
  mCodeParams.clear();
  mObsPgms.clear();

  fetchStations();
  fetchParams();
  fetchTypes();
}
