/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2018 met.no

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

#include "CachingAccess.hh"

#include "CachingAccessPrivate.hh"

#include "set_differences.hh"

#include <list>

#define MILOGGER_CATEGORY "kvhqc.CachingAccess"
#include "common/ObsLogging.hh"

// ========================================================================

BackendBuffer::BackendBuffer(const Sensor_s& sensors, const TimeSpan& timeSpan, ObsFilter_p filter)
    : TimeBuffer(std::make_shared<SignalRequest>(sensors, timeSpan, filter))
    , mUseCount(0)
    , mUnusedSince(timeutil::now())
{
  METLIBS_LOG_SCOPE(LOGVAL(sensors) << LOGVAL(timeSpan));
}

BackendBuffer::~BackendBuffer()
{
  METLIBS_LOG_SCOPE(LOGVAL(sensors()) << LOGVAL(timeSpan()) << LOGVAL(mUseCount));
}

void BackendBuffer::drop()
{
  METLIBS_LOG_SCOPE(LOGVAL(sensors()) << LOGVAL(timeSpan()) << LOGVAL(mUseCount));
  mUseCount -= 1;
  if (isUnused())
    mUnusedSince = timeutil::now();
}

BackendBuffer::obsrange_t BackendBuffer::findRange(const Sensor& sensor, const TimeSpan& time) const
{
  const ObsData_pv::const_iterator d_begin = data().begin(), d_end = data().end();
  const SensorTime st0(sensor, time.t0());
  ObsData_pv::const_iterator r_begin = std::lower_bound(d_begin, d_end, st0, OrderingHelper(ordering()));
  ObsData_pv::const_iterator r_end;
  if (r_begin != d_end && eq_Sensor()((*r_begin)->sensorTime().sensor, sensor)) {
    const SensorTime st1(sensor, time.t1());
    r_end = std::lower_bound(r_begin, d_end, st1, OrderingHelper(ordering()));
    if (r_end != d_end)
      ++r_end;
  } else {
    r_begin = r_end = d_end;
  }
  return std::make_pair(r_begin, r_end);
}

// ========================================================================

CacheTag::CacheTag(ObsRequest_p request, BackendBuffer_pv backendBuffers)
  : mRequest(request)
  , mBackendBuffers(backendBuffers)
  , mCountIncomplete(0)
  , mCountFailed(0)
{
  METLIBS_LOG_SCOPE();
  for (BackendBuffer_p bb : mBackendBuffers) {
    // TODO assert that the sensor matches sensor()

    bb->use();

    SignalRequest* sr = std::static_pointer_cast<SignalRequest>(bb->request()).get();
    connect(sr, &SignalRequest::requestCompleted, this, &CacheTag::onBackendCompleted);
    connect(sr, &SignalRequest::requestNewData, this, &CacheTag::onBackendNewData);
    connect(sr, &SignalRequest::requestUpdateData, this, &CacheTag::onBackendUpdateData);
    connect(sr, &SignalRequest::requestDropData, this, &CacheTag::onBackendDropData);

    const ObsData_pv bb_data = filterData(bb, true);
    if (!bb_data.empty())
      mRequest->newData(bb_data);

    METLIBS_LOG_DEBUG(LOGVAL(bb->status()));
    if (bb->status() == SimpleBuffer::INCOMPLETE) {
      mCountIncomplete += 1;
    } else if (bb->status() == SimpleBuffer::FAILED) {
      mCountFailed += 1;
    }
  }
  checkComplete();
}

CacheTag::~CacheTag()
{
  METLIBS_LOG_SCOPE(LOGVAL(mRequest->timeSpan()));
  for (BackendBuffer_p bb : mBackendBuffers) {
    bb->drop();
  }
}

ObsData_pv CacheTag::filterData(BackendBuffer_p bb, bool applyFilter)
{
  METLIBS_LOG_SCOPE();
  const Sensor_s& rsensors = mRequest->sensors();
  const bool validSensors = (rsensors.size() != 1 || rsensors.begin()->valid());
  const bool nofilter = !filter();
  METLIBS_LOG_DEBUG(LOGVAL(rsensors.size()) << LOGVAL(validSensors) << LOGVAL(applyFilter) << LOGVAL(nofilter));
  if ((!applyFilter || !filter()) && validSensors) {
    ObsData_pv dataOut;
    const TimeSpan& rtime = mRequest->timeSpan();
    for (const Sensor& rs : rsensors) {
      METLIBS_LOG_DEBUG(LOGVAL(rs));
      BackendBuffer::obsrange_t range = bb->findRange(rs, rtime);
      if (range.first != bb->data().end()) {
        METLIBS_LOG_DEBUG(LOGVAL((*range.first)->sensorTime()));
        if (range.second != bb->data().end())
          METLIBS_LOG_DEBUG(LOGVAL((*range.second)->sensorTime()));
        dataOut.insert(dataOut.end(), range.first, range.second);
      }
    }
    return dataOut;
  } else {
    return filterData(bb->data(), applyFilter);
  }
}

ObsData_pv CacheTag::filterData(const ObsData_pv& dataIn, bool applyFilter)
{
  METLIBS_LOG_SCOPE();
  ObsData_pv dataOut;

  const TimeSpan& rtime = mRequest->timeSpan();
  const Sensor_s& rsensors = mRequest->sensors();
  const bool validSensors = (rsensors.size() != 1 || rsensors.begin()->valid());

  for (ObsData_p obsI : dataIn) {
    if (!rtime.contains(obsI->sensorTime().time))
      continue;
    if (validSensors && rsensors.find(obsI->sensorTime().sensor) == rsensors.end())
      continue;

    if ((not applyFilter) or acceptFilter(obsI))
      // FIXME mData is a vector, insert one-by-one is not efficent
      dataOut.push_back(obsI);
  }
  return dataOut;
}

void CacheTag::checkComplete()
{
  METLIBS_LOG_SCOPE(LOGVAL(mCountIncomplete) << LOGVAL(mCountFailed));
  if (mCountIncomplete == 0) {
    ObsRequest_p reference = mRequest; // prevent deletion of request in complete handler
    reference->completed(mCountFailed ? QString("some tasks had errors") : QString());
  }
}

void CacheTag::onBackendCompleted(const QString& withError)
{
  METLIBS_LOG_SCOPE(LOGVAL(mCountIncomplete) << LOGVAL(mCountFailed));
  mCountIncomplete -= 1;
  if (not withError.isNull())
    mCountFailed += 1;
  checkComplete();
}

void CacheTag::onBackendNewData(const ObsData_pv& data)
{
  METLIBS_LOG_SCOPE();
  const ObsData_pv fdata = filterData(data, true);
  if (not fdata.empty())
    mRequest->newData(fdata);
}

void CacheTag::onBackendUpdateData(const ObsData_pv& data)
{
  METLIBS_LOG_SCOPE();
  const ObsData_pv fdata = filterData(data, false);
  if (not fdata.empty())
    mRequest->updateData(fdata);
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

  if (not droppedOut.empty())
    mRequest->dropData(droppedOut);
}

// ========================================================================

CachingAccessPrivate::CachingAccessPrivate(ObsAccess_p b)
  : backend(b)
{
}

CachingAccessPrivate::~CachingAccessPrivate()
{
  METLIBS_LOG_SCOPE();
  // clear cache is automatic
}

BackendBuffer_p CachingAccessPrivate::create(const Sensor_s& sensors, const TimeSpan& time, ObsFilter_p filter)
{
  METLIBS_LOG_SCOPE(LOGVAL(sensors) << LOGVAL(time));
  BackendBuffer_p bb = std::make_shared<BackendBuffer>(sensors, time, filter);
  BackendBuffer_pl::iterator it = std::upper_bound(mBuffers.begin(), mBuffers.end(), time.t0(), BackendBuffer_by_t0());
  mBuffers.insert(it, bb);
  return bb;
}

void CachingAccessPrivate::clean(const Time& dropBefore)
{
  METLIBS_LOG_SCOPE();
  for (BackendBuffer_pl::iterator itB = mBuffers.begin(); itB != mBuffers.end(); /*copy before increment*/) {
    BackendBuffer_pl::iterator itB_erase = itB++;
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
  METLIBS_LOG_SCOPE();
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
    return fa->subsetOf(fb) and fb->subsetOf(fa);
  return not (fa or fb);
}

// ========================================================================

class SensorTodo {
public:
  struct t0_closed {
    Time t0;
    bool closed;
    t0_closed(const Time& t)
      : t0(t), closed(false) { }
  };
  typedef std::map<Sensor, t0_closed, lt_Sensor> Sensor_todo;
  Sensor_todo todo;

  typedef std::map<Time, Sensor_s> Time_Sensors_m;

  SensorTodo(CachingAccessPrivate_p cap, const Sensor_s& sensors, ObsFilter_p filter, const Time& t0);

  void requestBuffers(const Sensor_s& intersection, const TimeSpan& time, bool close);

  void post();

  BackendBuffer_pv& shared()
    { return mToShare; }

private:
  /** group sensors in 'both' by needed TimeSpan to time.t0(), update
   * todo for all of them to time.t1()
   */
  Time_Sensors_m calculateRequestSpans(const Sensor_s& sensors, const TimeSpan& time, bool close);

private:
  CachingAccessPrivate_p mCAP;
  ObsFilter_p mFilter;
  BackendBuffer_pv mToPost, mToShare;
};

SensorTodo::SensorTodo(CachingAccessPrivate_p cap, const Sensor_s& sensors, ObsFilter_p filter, const Time& t0)
  : mCAP(cap)
  , mFilter(filter)
{
  const t0_closed tcinit(t0);
  for (const Sensor& s : sensors) {
    todo.insert(std::make_pair(s, tcinit));
  }
}

SensorTodo::Time_Sensors_m SensorTodo::calculateRequestSpans(const Sensor_s& sensors, const TimeSpan& time, bool close)
{
  METLIBS_LOG_SCOPE();
  Time_Sensors_m tsm;
  for (const Sensor& s : sensors) {
    Sensor_todo::iterator it = todo.find(s);
    if (it == todo.end()) {
      METLIBS_LOG_ERROR("sensor not in sensorTodo");
      continue;
    }
    t0_closed& tc = it->second;
    METLIBS_LOG_DEBUG(LOGVAL(tc.t0) << LOGVAL(tc.closed));
    if (time.t0() > tc.t0 or (close and not tc.closed and time.t0() == tc.t0))
      tsm[tc.t0].insert(s);
    tc.t0 = time.t1();
    tc.closed = true;
  }
  return tsm;
}

void SensorTodo::requestBuffers(const Sensor_s& intersection, const TimeSpan& time, bool close)
{
  METLIBS_LOG_SCOPE(LOGVAL(intersection) << LOGVAL(time));
  const Time_Sensors_m tsm = calculateRequestSpans(intersection, time, close);
  for (const Time_Sensors_m::value_type& ts : tsm) {
    const TimeSpan trequest(ts.first, time.t0());
    BackendBuffer_p bb = mCAP->create(ts.second, trequest, mFilter);
    mToShare.push_back(bb);
    mToPost .push_back(bb);
  }
}

void SensorTodo::post()
{
  for (BackendBuffer_p bb : mToPost) {
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

  for (BackendBuffer_p bb : p->mBuffers) {
    const TimeSpan& bbt = bb->timeSpan();
    if (bbt.t0() > rtime.t1()) {
      // bb starts after end of request; as backend buffers are sorted by time (see CachingAccessPrivate::create), we're done
      break;
    }
    if (bbt.t1() < rtime.t0()) {
      // bb ends before start of request; no intersection possible
      continue;
    }

    if (not filtersCompatible(rfilter, bb->filter())) {
      // filters are not compatible
      METLIBS_LOG_DEBUG("incompatible filters");
      continue;
    }

    Sensor_s intersection;
    typedef std::insert_iterator<Sensor_s> Sensor_si;
    std::set_intersection(rsensors.begin(), rsensors.end(),
        bb->sensors().begin(), bb->sensors().end(),
        Sensor_si(intersection,  intersection.begin()), lt_Sensor());

    // for the sensors in intersection, we make new requests for before the
    // buffer, and then put the buffer into the backendbuffer list
    if (not intersection.empty()) {
      todo.requestBuffers(intersection, bbt, false);

      // add the buffer for those we already had fetched
      todo.shared().push_back(bb);
    }
  }

  const Time& rt1 = request->timeSpan().t1();
  METLIBS_LOG_DEBUG(LOGVAL(rt1));
  todo.requestBuffers(rsensors, TimeSpan(rt1, rt1), true);

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

ObsUpdate_p CachingAccess::createUpdate(ObsData_p obs)
{
  return p->backend->createUpdate(obs);
}

bool CachingAccess::storeUpdates(const ObsUpdate_pv& updates)
{
  return p->backend->storeUpdates(updates);
}
