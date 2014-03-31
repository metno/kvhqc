
#include "CachingAccess.hh"

#include "CachingAccessPrivate.hh"

#include "set_differences.hh"

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

#include <list>

#define MILOGGER_CATEGORY "kvhqc.CachingAccess"
#include "common/ObsLogging.hh"

// ========================================================================

BackendBuffer::BackendBuffer(const Sensor_s& sensors, const TimeSpan& timeSpan, ObsFilter_p filter)
  : TimeBuffer(boost::make_shared<SignalRequest>(sensors, timeSpan, filter))
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
      const TimeBuffer::ObsDataByTime_ps& data = bb->data(); // FIXME avoid list/vector problem
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

BackendBuffer_p CachingAccessPrivate::create(const Sensor_s& sensors, const TimeSpan& time, ObsFilter_p filter)
{
  METLIBS_LOG_SCOPE(LOGVAL(sensors) << LOGVAL(time));
  BackendBuffer_p bb = boost::make_shared<BackendBuffer>(sensors, time, filter);
  mBuffers.insert(bb);
  return bb;
}

void CachingAccessPrivate::clean(const Time& dropBefore)
{
  METLIBS_LOG_SCOPE();
  for (Time0_Buffer_s::iterator itB = mBuffers.begin(); itB != mBuffers.end(); /*copy before increment*/) {
    Time0_Buffer_s::iterator itB_erase = itB++;
    BackendBuffer_p bb = *itB_erase;

    if (bb->isUnused() and bb->unusedSince() < dropBefore) {
      METLIBS_LOG_DEBUG("drop provider buffer " << bb->sensors() << " " << bb->timeSpan());
      mBuffers.erase(itB_erase);
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

// ========================================================================

namespace /*anonymous*/ {

bool filtersCompatible(ObsFilter_p fa, ObsFilter_p fb)
{
  if (fa and fb)
    return fa->subsetOf(*fb);
  return not (fa or fb);
}

// ========================================================================

class SensorTodo {
public:
  typedef std::pair<Time, bool> t0_closed;
  typedef std::map<Sensor, t0_closed, lt_Sensor> Sensor_todo;
  Sensor_todo todo;

  typedef std::map<Time, Sensor_s> Time_Sensors_m;
  
  SensorTodo(CachingAccessPrivate_p cap, const Sensor_s& sensors, ObsFilter_p filter, const Time& t0);

  void requestBuffers(const Sensor_s& intersection, const TimeSpan& time);

  void post();

  BackendBuffer_pv& shared()
    { return mToShare; }

private:
  /** group sensors in 'both' by needed TimeSpan to time.t0(), update
   * todo for all of them to time.t1()
   */
  Time_Sensors_m calculateRequestSpans(const Sensor_s& sensors, const TimeSpan& time);

private:
  CachingAccessPrivate_p mCAP;
  ObsFilter_p mFilter;
  BackendBuffer_pv mToPost, mToShare;
};

SensorTodo::SensorTodo(CachingAccessPrivate_p cap, const Sensor_s& sensors, ObsFilter_p filter, const Time& t0)
  : mCAP(cap)
  , mFilter(filter)
{
  const t0_closed tcinit(t0, false);
  BOOST_FOREACH(const Sensor& s, sensors) {
    todo.insert(std::make_pair(s, tcinit));
  }
}

SensorTodo::Time_Sensors_m SensorTodo::calculateRequestSpans(const Sensor_s& sensors, const TimeSpan& time)
{
  Time_Sensors_m tsm;
  BOOST_FOREACH(const Sensor& s, sensors) {
    Sensor_todo::iterator it = todo.find(s);
    if (it == todo.end()) {
      METLIBS_LOG_ERROR("sensor not in sensorTodo");
      continue;
    }
    t0_closed& tc = it->second;
    if (time.t0() > tc.first)
      tsm[tc.first].insert(s);
    tc.first = time.t1();
    tc.second = true;
  }
  return tsm;
}

void SensorTodo::requestBuffers(const Sensor_s& intersection, const TimeSpan& time)
{
  METLIBS_LOG_SCOPE(LOGVAL(intersection) << LOGVAL(time));
  const Time_Sensors_m tsm = calculateRequestSpans(intersection, time);
  BackendBuffer_pl buffers;
  BOOST_FOREACH(const Time_Sensors_m::value_type& ts, tsm) {
    const TimeSpan trequest(ts.first, time.t0());
    BackendBuffer_p bb = mCAP->create(ts.second, trequest, mFilter);
    mToShare.push_back(bb);
    mToPost .push_back(bb);
  }
}

void SensorTodo::post()
{
  BOOST_FOREACH(BackendBuffer_p bb, mToPost) {
    bb->postRequest(mCAP->backend);
  }
}

} // namespace anonymous

// ========================================================================

void CachingAccess::postRequest(ObsRequest_p request)
{
  METLIBS_LOG_SCOPE();
  const Sensor_s& rsensors = request->sensors();
  const TimeSpan& rtime    = request->timeSpan();
  ObsFilter_p     rfilter  = request->filter();

  SensorTodo todo(p, rsensors, rfilter, rtime.t0());

  // TODO throw if time is not closed()

  // may not post request before creating CacheTag, otherwise counting
  // of incomplete buffers cannot work

  BOOST_FOREACH(BackendBuffer_p bb, p->mBuffers) {
    // continue if filters do not match
    if (not filtersCompatible(rfilter, bb->filter())) {
      METLIBS_LOG_DEBUG("incompatible filters");
      continue;
    }

    const TimeSpan& bbt = bb->timeSpan();
    METLIBS_LOG_DEBUG(LOGVAL(bbt));

    if (bbt.t0() > rtime.t1())
      break;

    Sensor_s intersection;
    typedef std::insert_iterator<Sensor_s> Sensor_si;
    std::set_intersection(rsensors.begin(), rsensors.end(),
        bb->sensors().begin(), bb->sensors().end(),
        Sensor_si(intersection,  intersection.begin()), lt_Sensor());

    // for those in intersection, we make new requests for before the
    // buffer, and then put the buffer into the backendbuffer list
    if (not intersection.empty()) {
      todo.requestBuffers(intersection, bbt);

      // add the buffer for those we already had fetched
      todo.shared().push_back(bb);

      if (bbt.t1() >= rtime.t1())
        break;
    }
  }

  const Time& rt1 = request->timeSpan().t1();
  todo.requestBuffers(rsensors, TimeSpan(rt1, rt1));

  request->setTag(new CacheTag(request, todo.shared()));

  todo.post();
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
