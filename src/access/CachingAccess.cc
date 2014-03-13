
#include "CachingAccess.hh"

#include "CachingAccessPrivate.hh"

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

#include <list>

#define MILOGGER_CATEGORY "kvhqc.CachingAccess"
#include "common/ObsLogging.hh"

// ========================================================================

BackendBuffer::BackendBuffer(const Sensor& sensor, const TimeSpan& timeSpan, ObsFilter_p filter)
  : TimeBuffer(boost::make_shared<SignalRequest>(sensor, timeSpan, filter))
  , mUseCount(0)
  , mUnusedSince(timeutil::now())
{
  METLIBS_LOG_SCOPE();
}

BackendBuffer::~BackendBuffer()
{
  METLIBS_LOG_SCOPE();
}

void BackendBuffer::drop()
{
  METLIBS_LOG_SCOPE(LOGVAL(mUseCount));
  mUseCount -= 1;
  if (isUnused())
    mUnusedSince = timeutil::now();
}

// ========================================================================

CacheTag::CacheTag(ObsRequest_p request, BackendBuffer_pv backendBuffers)
  : mRequest(request)
  , mBackendBuffers(backendBuffers)
  , mCountIncomplete(0)
  , mCountFailed(0)
{
  METLIBS_LOG_SCOPE();
  BOOST_FOREACH(BackendBuffer_p bb, mBackendBuffers) {
    // TODO assert that the sensor matches sensor()
  
    bb->use();

    SignalRequest* sr = boost::static_pointer_cast<SignalRequest>(bb->request()).get();
    connect(sr, SIGNAL(requestCompleted(bool)),
        this,   SLOT(onBackendCompleted(bool)));
    connect(sr, SIGNAL(requestNewData(const ObsData_pv&)),
        this,   SLOT(onBackendNewData(const ObsData_pv&)));
    connect(sr, SIGNAL(requestUpdateData(const ObsData_pv&)),
        this,   SLOT(onBackendUpdateData(const ObsData_pv&)));
    connect(sr, SIGNAL(requestDropData(const SensorTime_v&)),
        this,   SLOT(onBackendDropData(const SensorTime_v&)));
    
    METLIBS_LOG_DEBUG(LOGVAL(bb->status()));
    if (bb->status() == SimpleBuffer::COMPLETE) {
      const ObsData_pl& data = bb->data(); // FIXME avoid list/vector problem
      mRequest->newData(filterData(ObsData_pv(data.begin(), data.end())));
    } else {
      mCountIncomplete += 1;
      if (bb->status() == SimpleBuffer::FAILED)
        mCountFailed += 1;
    }
  }
  checkComplete();
}

CacheTag::~CacheTag()
{
  METLIBS_LOG_SCOPE();
  BOOST_FOREACH(BackendBuffer_p bb, mBackendBuffers) {
    bb->drop();
  }
}

bool CacheTag::acceptObs(ObsData_p obs) const
{
  return mRequest->timeSpan().contains(obs->sensorTime().time)
      and acceptFilter(obs);
}

ObsData_pv CacheTag::filterData(const ObsData_pv& dataIn)
{
  METLIBS_LOG_SCOPE();
  ObsData_pv dataOut;

  const TimeSpan& rtime = mRequest->timeSpan();

  for(ObsData_pv::const_iterator itI = dataIn.begin(); itI != dataIn.end(); ++itI) {
    ObsData_p obsI = (*itI);
    const Time& timeI = obsI->sensorTime().time;
    if (timeI < rtime.t0())
      continue;
    if (timeI > rtime.t1())
      break;

    if (acceptFilter(obsI))
      // FIXME mData is a vector, insert one-by-one is not efficent
      dataOut.push_back(obsI);
  }
  return dataOut;
}

void CacheTag::checkComplete()
{
  METLIBS_LOG_SCOPE(LOGVAL(mCountIncomplete) << LOGVAL(mCountFailed));
  if (mCountIncomplete == 0)
    mRequest->completed(mCountFailed != 0);
}

void CacheTag::onBackendCompleted(bool failed)
{
  METLIBS_LOG_SCOPE(LOGVAL(mCountIncomplete) << LOGVAL(mCountFailed));
  mCountIncomplete -= 1;
  if (failed)
    mCountFailed += 1;
  checkComplete();
}

void CacheTag::onBackendNewData(const ObsData_pv& data)
{
  METLIBS_LOG_SCOPE();
  mRequest->newData(filterData(data));
}

void CacheTag::onBackendUpdateData(const ObsData_pv& data)
{
  METLIBS_LOG_SCOPE();
  mRequest->updateData(filterData(data));
}

void CacheTag::onBackendDropData(const SensorTime_v& droppedIn)
{
  METLIBS_LOG_SCOPE();
  
  SensorTime_v droppedOut;
  const TimeSpan& rtime = mRequest->timeSpan();
  for(SensorTime_v::const_iterator itI = droppedIn.begin(); itI != droppedIn.end(); ++itI) {
    const Time& timeI = itI->time;
    if (timeI < rtime.t0())
      continue;
    if (timeI > rtime.t1())
      break;

    droppedOut.push_back(*itI);
  }

  mRequest->dropData(droppedOut);
}

// ========================================================================

CachingAccessPrivate::CachingAccessPrivate(ObsAccess_p b)
  : backend(b)
{
}

CachingAccessPrivate::~CachingAccessPrivate()
{
  // clear cache is automatic
}

BackendBuffer_p CachingAccessPrivate::create(ObsRequest_p request, const TimeSpan& time)
{
  METLIBS_LOG_SCOPE(LOGVAL(request->sensor()) << LOGVAL(time));
  BackendBuffer_p bb = boost::make_shared<BackendBuffer>(request->sensor(), time, request->filter());
  mSensorBuffers[request->sensor()].insert(bb);
  return bb;
}

void CachingAccessPrivate::clean(const Time& dropBefore)
{
  METLIBS_LOG_SCOPE();
  for (Sensor_Buffer_m::iterator itS = mSensorBuffers.begin(); itS != mSensorBuffers.end(); ) {
    Sensor_Buffer_m::iterator itS_erase = itS++;
    Buffer_s& buffers = itS_erase->second;
    
    for (Buffer_s::iterator itB = buffers.begin(); itB != buffers.end(); ) {
      Buffer_s::iterator itB_erase = itB++;
      BackendBuffer_p bb = *itB_erase;

      if (bb->isUnused() and bb->unusedSince() < dropBefore) {
        METLIBS_LOG_DEBUG("drop provider buffer " << bb->sensor() << " " << bb->timeSpan());
        buffers.erase(itB_erase);
      }
    }
    if (buffers.empty()) {
      METLIBS_LOG_DEBUG("erase for " << itS_erase->first);
      mSensorBuffers.erase(itS_erase);
    }
  }
}

// ========================================================================

CachingAccess::CachingAccess(ObsAccess_p backend)
  : p(new CachingAccessPrivate(backend))
{
}

CachingAccess::~CachingAccess()
{
}

void CachingAccess::cleanCache(const Time& dropBefore)
{
  p->clean(dropBefore);
}

static bool filtersCompatible(ObsFilter_p fa, ObsFilter_p fb)
{
  if (fa and fb and not fa->equals(*fb))
    return false;
  return not (fa or fb);
}

void CachingAccess::postRequest(ObsRequest_p request)
{
  METLIBS_LOG_SCOPE();
  const Sensor& sensor = request->sensor();
  const TimeSpan& time = request->timeSpan();
  ObsFilter_p filter   = request->filter();
  METLIBS_LOG_DEBUG(LOGVAL(sensor) << LOGVAL(time));

  // TODO throw if time is not closed()

  Time todo = time.t0();
  bool todoClosed = false;

  BackendBuffer_pv bbs;
  BackendBuffer_pv toPost;

  BOOST_FOREACH(BackendBuffer_p bb, p->mSensorBuffers[sensor]) {
    // continue if filters do not match
    if (not filtersCompatible(filter, bb->filter())) {
      METLIBS_LOG_DEBUG("incompatible filters");
      continue;
    }

    const TimeSpan& bbt = bb->timeSpan();
    METLIBS_LOG_DEBUG(LOGVAL(bbt) << LOGVAL(todo));

    if (bbt.t0() > todo and bbt.t0() <= time.t1()) {
      BackendBuffer_p bb = p->create(request, TimeSpan(todo, time.t1()));
      bbs.push_back(bb);
      toPost.push_back(bb);
      todo = bbt.t1();
      todoClosed = true;
    }
    if (bbt.t0() <= todo and bbt.t1() >= todo) {
      bbs.push_back(bb);
      todo = bbt.t1();
      todoClosed = true;
      if (todo >= time.t1())
        break;
    }
  }

  METLIBS_LOG_DEBUG(LOGVAL(todo) << LOGVAL(todoClosed));
  if ((todoClosed and todo < time.t1())
      or ((not todoClosed) and todo <= time.t1()))
  {
    BackendBuffer_p bb = p->create(request, TimeSpan(todo, time.t1()));
    bbs.push_back(bb);
    toPost.push_back(bb);
  }

  request->setTag(new CacheTag(request, bbs));
  BOOST_FOREACH(BackendBuffer_p bb, toPost) {
    bb->postRequest(p->backend);
  }
}

void CachingAccess::dropRequest(ObsRequest_p request)
{
  METLIBS_LOG_SCOPE();
  CacheTag_x tag = static_cast<CacheTag_x>(request->tag());
  delete tag;
}

ObsUpdate_p CachingAccess::createUpdate(const SensorTime& sensorTime)
{
  return p->backend->createUpdate(sensorTime);
}

bool CachingAccess::storeUpdates(const ObsUpdate_pv& updates)
{
  return p->backend->storeUpdates(updates);
}
