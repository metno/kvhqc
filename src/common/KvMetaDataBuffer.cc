
#include "KvMetaDataBuffer.hh"

#include "Functors.hh"
#include "HqcSystemDB.hh"
#include "HqcApplication.hh"
#include "KvHelpers.hh"
#include "ParamQueryTask.hh"
#include "QueryTaskHelper.hh"
#include "StationQueryTask.hh"
#include "TypesQueryTask.hh"

#include <puTools/miStringBuilder.h>
#include <puTools/miStringFunctions.h>

#define MILOGGER_CATEGORY "kvhqc.KvMetaDataBuffer"
#include "common/ObsLogging.hh"

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
  dropStations();
  dropParams();
  dropTypes();

  sInstance = 0;
}

bool KvMetaDataBuffer::isKnownStation(int id)
{
  const hqc::kvStation_v& all = allStations();
  hqc::kvStation_v::const_iterator it = std::lower_bound(all.begin(), all.end(), id, Helpers::kvStation_lt_id());
  return (it != all.end() and it->stationID() == id);
}

const kvalobs::kvStation& KvMetaDataBuffer::findStation(int id)
{
  const hqc::kvStation_v& all = allStations();
  hqc::kvStation_v::const_iterator it = std::lower_bound(all.begin(), all.end(), id, Helpers::kvStation_lt_id());
  if (it != all.end() and it->stationID() == id)
    return *it;
  throw std::runtime_error(miutil::StringBuilder() << "station " << id << "' not found");
}

const std::vector<kvalobs::kvStation>& KvMetaDataBuffer::allStations()
{
  static hqc::kvStation_v empty;
  if (mTaskStations and mHaveStations)
    return static_cast<const StationQueryTask*>(mTaskStations->task())->stations();
  return empty;
}

bool KvMetaDataBuffer::isKnownParam(int id)
{
  const hqc::kvParam_v& all = allParams();
  hqc::kvParam_v::const_iterator it = std::lower_bound(all.begin(), all.end(), id, Helpers::kvParam_lt_id());
  return (it != all.end() and it->paramID() == id);
}

const kvalobs::kvParam& KvMetaDataBuffer::findParam(int id)
{
  const hqc::kvParam_v& all = allParams();
  hqc::kvParam_v::const_iterator it = std::lower_bound(all.begin(), all.end(), id, Helpers::kvParam_lt_id());
  if (it != all.end() and it->paramID() == id)
    return *it;
  throw std::runtime_error(miutil::StringBuilder() << "param " << id << "' not found");
}

const std::vector<kvalobs::kvParam>& KvMetaDataBuffer::allParams()
{
  static hqc::kvParam_v empty;
  if (mTaskParams and mHaveParams)
    return static_cast<const ParamQueryTask*>(mTaskParams->task())->params();
  return empty;
}

std::string KvMetaDataBuffer::findParamName(int id)
{
  const hqc::kvParam_v& all = allParams();
  hqc::kvParam_v::const_iterator it = std::lower_bound(all.begin(), all.end(), id, Helpers::kvParam_lt_id());
  if (it != all.end() and it->paramID() == id)
    return it->name();
  
  return miutil::from_number(id);
}

bool KvMetaDataBuffer::isCodeParam(int paramid)
{
  return mCodeParams.find(paramid) != mCodeParams.end();
}

bool KvMetaDataBuffer::isDirectionInDegreesParam(int paramid)
{
  return mDirectionParams.find(paramid) != mDirectionParams.end();
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
  const hqc::kvTypes_v& all = allTypes();
  hqc::kvTypes_v::const_iterator it = std::lower_bound(all.begin(), all.end(), id, Helpers::kvTypes_lt_id());
  return (it != all.end() and it->typeID() == id);
}

const kvalobs::kvTypes& KvMetaDataBuffer::findType(int id)
{
  const hqc::kvTypes_v& all = allTypes();
  hqc::kvTypes_v::const_iterator it = std::lower_bound(all.begin(), all.end(), id, Helpers::kvTypes_lt_id());
  if (it != all.end() and it->typeID() == id)
    return *it;
  throw std::runtime_error(miutil::StringBuilder() << "type " << id << "' not found");
}

const hqc::kvTypes_v& KvMetaDataBuffer::allTypes()
{
  static hqc::kvTypes_v empty;
  if (mTaskTypes and mHaveTypes)
    return static_cast<const TypesQueryTask*>(mTaskTypes->task())->types();
  return empty;
}

const hqc::kvObsPgm_v& KvMetaDataBuffer::findObsPgm(int stationid)
{
  static hqc::kvObsPgm_v empty;

  const kvObsPgm_m::iterator it = mObsPgms.find(stationid);
  if (it != mObsPgms.end())
    return it->second;

  return empty;
}

void KvMetaDataBuffer::putObsPgm(const hqc::kvObsPgm_v& op)
{
  for (hqc::kvObsPgm_v::const_iterator it = op.begin(); it != op.end(); ++it)
    mObsPgms[it->stationID()].push_back(*it);
}

void KvMetaDataBuffer::taskDoneStations()
{
  mHaveStations = true;
  sendComplete();
}

void KvMetaDataBuffer::taskDoneParams()
{
  mHaveParams = true;

  mCodeParams.clear();
  const hqc::kvParam_v& ap = allParams();
  for (hqc::kvParam_v::const_iterator it = ap.begin(); it != ap.end(); ++it) {
    const int pid = it->paramID();
    if (it->unit().find("kode") != std::string::npos)
      mCodeParams.insert(pid);
    if (it->unit().find("grader") != std::string::npos
        and pid != kvalobs::PARAMID_MLON
        and pid != kvalobs::PARAMID_MLAT)
    {
      mDirectionParams.insert(pid);
    }
  }

  sendComplete();
}

void KvMetaDataBuffer::taskDoneTypes()
{
  mHaveTypes = true;
  sendComplete();
}

void KvMetaDataBuffer::sendComplete()
{
  if (isComplete())
    Q_EMIT complete();
}

QueryTaskHandler_p KvMetaDataBuffer::handler()
{
  if (mHandler)
    return mHandler;
  else if (hqcApp)
    return hqcApp->kvalobsHandler();
  else
    return QueryTaskHandler_p();
}

void KvMetaDataBuffer::fetchStations()
{
  METLIBS_LOG_SCOPE();
  dropStations();

  StationQueryTask* q = new StationQueryTask(QueryTask::PRIORITY_AUTOMATIC);
  mTaskStations = new QueryTaskHelper(q);
  connect(mTaskStations, SIGNAL(done(SignalTask*)), this, SLOT(taskDoneStations()));
  mTaskStations->post(handler().get());
}

void KvMetaDataBuffer::fetchParams()
{
  METLIBS_LOG_SCOPE();
  dropParams();

  ParamQueryTask* q = new ParamQueryTask(QueryTask::PRIORITY_AUTOMATIC);
  mTaskParams = new QueryTaskHelper(q);
  connect(mTaskParams, SIGNAL(done(SignalTask*)), this, SLOT(taskDoneParams()));
  mTaskParams->post(handler().get());
}

void KvMetaDataBuffer::fetchTypes()
{
  METLIBS_LOG_SCOPE();
  dropTypes();

  TypesQueryTask* q = new TypesQueryTask(QueryTask::PRIORITY_AUTOMATIC);
  mTaskTypes = new QueryTaskHelper(q);
  connect(mTaskTypes, SIGNAL(done(SignalTask*)), this, SLOT(taskDoneTypes()));
  mTaskTypes->post(handler().get());
}

void KvMetaDataBuffer::dropStations()
{
  delete mTaskStations;
  mTaskStations = 0;
}

void KvMetaDataBuffer::dropParams()
{
  delete mTaskParams;
  mTaskParams = 0;
}

void KvMetaDataBuffer::dropTypes()
{
  delete mTaskTypes;
  mTaskTypes = 0;
}

void KvMetaDataBuffer::reload()
{
  mHaveStations = mHaveParams = mHaveTypes = false;
  mCodeParams.clear();
  mDirectionParams.clear();
  mObsPgms.clear();

  fetchStations();
  fetchParams();
  fetchTypes();
}
