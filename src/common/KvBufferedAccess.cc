
#include "KvBufferedAccess.hh"

#include "KvHelpers.hh"
#include "KvalobsData.hh"

#include <kvalobs/kvDataOperations.h>

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

#define MILOGGER_CATEGORY "kvhqc.KvBufferedAccess"
#include "common/ObsLogging.hh"

ObsAccess::TimeSet KvBufferedAccess::allTimes(const std::vector<Sensor>& sensors, const TimeRange& limits)
{
  TimeSet times;

  BOOST_FOREACH(const Sensor& sensor, sensors) {
    for (Data_t::const_iterator it = mData.lower_bound(SensorTime(sensor, limits.t0())); it != mData.end(); ++it)
    {
      const SensorTime& dst = it->first;
      if ((not eq_Sensor()(dst.sensor, sensor)) or (dst.time > limits.t1()))
        break;
      if (it->second)
        times.insert(dst.time);
    }
  }
  return times;
}

ObsAccess::DataSet KvBufferedAccess::allData(const std::vector<Sensor>& sensors, const TimeRange& limits)
{
  DataSet data;
  
  BOOST_FOREACH(const Sensor& sensor, sensors) {
    for (Data_t::const_iterator it = mData.lower_bound(SensorTime(sensor, limits.t0())); it != mData.end(); ++it) {
      const SensorTime& dst = it->first;
      if ((not eq_Sensor()(dst.sensor, sensor)) or (dst.time > limits.t1()))
        break;
      if (it->second)
        data.insert(it->second);
    }
  }
  return data;
}

ObsDataPtr KvBufferedAccess::find(const SensorTime& st)
{
  if (not st.valid()) {
    HQC_LOG_ERROR("invalid sensorTime: " << st);
    return ObsDataPtr();
  }

  Data_t::iterator it = mData.find(st);
  if (it != mData.end())
    return it->second;
  
  return KvalobsDataPtr();
}

ObsDataPtr KvBufferedAccess::create(const SensorTime& st)
{
  if (not st.valid())
    HQC_LOG_ERROR("invalid sensorTime: " << st);
  
  Data_t::iterator it = mData.find(st);
  if (it != mData.end() and it->second)
    return it->second;
  
  const Sensor& s = st.sensor;
  kvalobs::kvData d = kvalobs::getMissingKvData(s.stationId, timeutil::to_miTime(st.time),
      s.paramId, s.typeId, s.sensor, s.level);
  d.corrected(kvalobs::NEW_ROW);
  KvalobsDataPtr obs = boost::make_shared<KvalobsData>(d, true);
  mData[st] = obs;
  obsDataChanged(CREATED, obs);
  return obs;
}

bool KvBufferedAccess::updatesHaveTasks(const std::vector<ObsUpdate>& updates)
{
  BOOST_FOREACH(const ObsUpdate& ou, updates) {
    if (ou.tasks != 0)
      return true;
  }
  return false;
}

void KvBufferedAccess::receive(const kvalobs::kvData& data, bool update)
{
  const SensorTime st(Helpers::sensorTimeFromKvData(data));
  Data_t::iterator it = mData.lower_bound(st);
  if (it == mData.end() or not eq_SensorTime()(st, it->first)) {
    if (isSubscribed(st)) {
      KvalobsDataPtr obs = boost::make_shared<KvalobsData>(data, false);
      mData.insert(it, std::make_pair(st, obs));
      if (update)
        obsDataChanged(CREATED, obs);
    }
  } else {
    KvalobsDataPtr obs = it->second;
    const bool hadBeenCreated = obs->isCreated();
    obs->setCreated(false);

    // FIXME this might compare too many things ...
    const bool hasChanged = not kvalobs::compare::exactly_equal()(obs->data(), data);
    if (hasChanged)
      obs->data() = data;
    if (hadBeenCreated or hasChanged)
      obsDataChanged(MODIFIED, obs);
  }
}

bool KvBufferedAccess::drop(const SensorTime& st)
{
  Data_t::iterator it = mData.find(st);
  if (it == mData.end())
    return false;

  ObsDataPtr obs = it->second;
  mData.erase(it);
  obsDataChanged(ObsAccess::DESTROYED, obs);
  return true;
}

void KvBufferedAccess::addSubscription(const ObsSubscription& s)
{
  METLIBS_LOG_SCOPE();
  METLIBS_LOG_DEBUG(LOGVAL(s.stationId()) << LOGVAL(s.time()));

  if (not s.time().closed()) {
    HQC_LOG_WARN("trying to subscribe for non-closed time span " << s.time());
    return;
  }
  mSubscriptions.push_back(s);

  SubscribedTimes_t::iterator it = mSubscribedTimes.find(s.stationId());
  if (it != mSubscribedTimes.end()) {
    // FIXME merge only overlapping time spans
    TimeRange r(std::min(s.time().t0(), it->second.t0()), std::max(s.time().t1(), it->second.t1()));
    METLIBS_LOG_DEBUG(LOGVAL(it->second) << LOGVAL(r));
    it->second = r;
  } else {
    mSubscribedTimes.insert(SubscribedTimes_t::value_type(s.stationId(), s.time()));
  }
}

namespace {
struct eq_ObsSubscription : public std::unary_function<bool, ObsSubscription> {
  const ObsSubscription& b;
  eq_ObsSubscription(const ObsSubscription& os) : b(os) { }
  bool operator()(const ObsSubscription& a) const
    {
      return a.stationId() == b.stationId()
          and a.time().t0() == b.time().t0()
          and a.time().t1() == b.time().t1();
    }
};
}

void KvBufferedAccess::removeSubscription(const ObsSubscription& s)
{
  METLIBS_LOG_SCOPE();
  METLIBS_LOG_DEBUG(LOGVAL(s.stationId()) << LOGVAL(s.time()));

  if (not s.time().closed()) {
    HQC_LOG_WARN("trying to unsubscribe for non-closed time span " << s.time());
    return;
  }
  
  Subscriptions_t::iterator it_sub = std::find_if(mSubscriptions.begin(), mSubscriptions.end(), eq_ObsSubscription(s));
  if (it_sub == mSubscriptions.end()) {
    HQC_LOG_ERROR("subscribed station " << s.stationId() << " not in subscription list");
    return;
  }
  mSubscriptions.erase(it_sub);
      
  // FIXME this relies on using a simplified (ie without gaps) time list for each station
  SubscribedTimes_t::iterator it = mSubscribedTimes.find(s.stationId());
  assert(it != mSubscribedTimes.end());
  METLIBS_LOG_DEBUG(LOGVAL(it->second));
  
  if (s.time().t0() == it->second.t0() or s.time().t1() == it->second.t1()) {
    // at start/end, need to scan all subscriptions for this station
      
    TimeRange newSpan;
    BOOST_FOREACH(const ObsSubscription& sub, mSubscriptions) {
      if (sub.stationId() != s.stationId())
        continue;
      
      if (newSpan.closed()) {
        // FIXME this merges across gaps
        newSpan = TimeRange(std::min(sub.time().t0(), newSpan.t0()), std::max(sub.time().t1(), newSpan.t1()));
      } else {
        newSpan = sub.time();
      }
    }
    METLIBS_LOG_DEBUG(LOGVAL(newSpan));
    if (newSpan.closed())
      it->second = newSpan;
    else
      mSubscribedTimes.erase(it);
  }
}

bool KvBufferedAccess::isSubscribed(const SensorTime& st)
{
  SubscribedTimes_t::iterator it = mSubscribedTimes.find(st.sensor.stationId);
  if (it == mSubscribedTimes.end())
    return false;
  // FIXME this is not exact, it returns true also for times in a "gap" between two subscriptions
  return it->second.contains(st.time);
}
