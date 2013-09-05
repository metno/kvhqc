
#include "KvBufferedAccess.hh"
#include "Helpers.hh"
#include "KvalobsData.hh"

#include <kvalobs/kvDataOperations.h>

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

#define MILOGGER_CATEGORY "kvhqc.KvBufferedAccess"
#include "HqcLogging.hh"

ObsAccess::TimeSet KvBufferedAccess::allTimes(const Sensor& sensor, const TimeRange& limits)
{
    TimeSet times;

    for (Data_t::const_iterator it = mData.lower_bound(SensorTime(sensor, limits.t0()));
         it != mData.end(); ++it)
    {
      const SensorTime& dst = it->first;
      if ((not eq_Sensor()(dst.sensor, sensor)) or (dst.time > limits.t1()))
        break;
      if (it->second)
        times.insert(dst.time);
    }
    return times;
}

ObsAccess::DataSet KvBufferedAccess::allData(const Sensor& sensor, const TimeRange& limits)
{
  DataSet data;

  for (Data_t::const_iterator it = mData.lower_bound(SensorTime(sensor, limits.t0())); it != mData.end(); ++it) {
    const SensorTime& dst = it->first;
    if ((not eq_Sensor()(dst.sensor, sensor)) or (dst.time > limits.t1()))
      break;
    if (it->second)
      data.insert(it->second);
  }
  return data;
}

ObsDataPtr KvBufferedAccess::find(const SensorTime& st)
{
    if (not st.valid()) {
        METLIBS_LOG_ERROR("invalid sensorTime: " << st);
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
        METLIBS_LOG_ERROR("invalid sensorTime: " << st);

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

KvalobsDataPtr KvBufferedAccess::receive(const kvalobs::kvData& data)
{
    const SensorTime st(Helpers::sensorTimeFromKvData(data));
    if (not isSubscribed(st)) {
        METLIBS_LOG_DEBUG("no subscription for " << data);
        return KvalobsDataPtr();
    }
    
    KvalobsDataPtr obs;
    Data_t::iterator it = mData.lower_bound(st);
    if (it == mData.end() or not eq_SensorTime()(st, it->first)) {
        obs = boost::make_shared<KvalobsData>(data, false);
        mData.insert(it, std::make_pair(st, obs));
        obsDataChanged(CREATED, obs);
    } else {
        obs = it->second;

        // FIXME this might compare too many things ...
        if (not kvalobs::compare::exactly_equal()(obs->data(), data)) {
            obs->data() = data;
            obsDataChanged(MODIFIED, obs);
        }
    }
    return obs;
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
    if (s.time().t0().is_not_a_date_time() or s.time().t1().is_not_a_date_time())
        return;
    mSubscriptions.push_back(s);

    SubscribedTimes_t::iterator it = mSubscribedTimes.find(s.stationId());
    if (it != mSubscribedTimes.end()) {
      // FIXME merge only overlapping time spans
      TimeRange r(std::min(s.time().t0(), it->second.t0()), std::max(s.time().t1(), it->second.t1()));
      it->second = r;
    } else {
      mSubscribedTimes.insert(SubscribedTimes_t::value_type(s.stationId(), s.time()));
    }
}

namespace {
bool operator==(const ObsSubscription& a, const ObsSubscription& b)
{
    return a.stationId() == b.stationId()
        and a.time().t0() == b.time().t0()
        and a.time().t1() == b.time().t1();
}
bool operator!=(const ObsSubscription& a, const ObsSubscription& b)
{
    return a.stationId() != b.stationId()
        or a.time().t0() != b.time().t0()
        or a.time().t1() != b.time().t1();
}

struct eq_ObsSubscription : public std::unary_function<bool, ObsSubscription> {
    const ObsSubscription& b;
    eq_ObsSubscription(const ObsSubscription& os) : b(os) { }
    bool operator()(const ObsSubscription& a) const
        { return a == b; }
};
}

void KvBufferedAccess::removeSubscription(const ObsSubscription& s)
{
  if (s.time().t0().is_not_a_date_time() or s.time().t1().is_not_a_date_time())
    return;
  
  Subscriptions_t::iterator it = std::find_if(mSubscriptions.begin(), mSubscriptions.end(), eq_ObsSubscription(s));
  //Subscriptions_t::iterator it = std::find(mSubscriptions.begin(), mSubscriptions.end(), s);
  if (it != mSubscriptions.end()) {
    mSubscriptions.erase(it);
      
    // FIXME this relies on using a simplified (ie without gaps) time list for each station
    SubscribedTimes_t::iterator it = mSubscribedTimes.find(s.stationId());
    if (it != mSubscribedTimes.end()) {
      if (s.time().t0() == it->second.t0() or s.time().t1() == it->second.t1()) {
        // at start/end, need to scan all subscriptions for this station
          
        TimeRange newSpan;
        BOOST_FOREACH(const ObsSubscription& sub, mSubscriptions) {
          if (sub.stationId() != s.stationId())
            continue;
            
          if (newSpan.undef()) {
            // FIXME this does not merge across gaps
            newSpan = TimeRange(std::min(sub.time().t0(), newSpan.t0()), std::max(sub.time().t1(), newSpan.t1()));
          } else {
            newSpan = sub.time();
          }
        }
        it->second = newSpan;
      }
    } else {
      METLIBS_LOG_ERROR("subscribed station " << s.stationId() << " not in subscription list");
    }
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
