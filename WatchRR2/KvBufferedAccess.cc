
#include "KvBufferedAccess.hh"
#include "Helpers.hh"
#include "KvalobsData.hh"

#include <kvalobs/kvDataOperations.h>

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

#define NDEBUG
#include "w2debug.hh"

ObsDataPtr KvBufferedAccess::find(const SensorTime& st)
{
    Data_t::iterator it = mData.find(st);
    if (it != mData.end())
        return it->second;

    return KvalobsDataPtr();
}

ObsDataPtr KvBufferedAccess::create(const SensorTime& st)
{
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

bool KvBufferedAccess::update(const std::vector<ObsUpdate>& updates)
{
    // reject anything with tasks
    if (updatesHaveTasks(updates))
        return false;

    BOOST_FOREACH(const ObsUpdate& ou, updates) {
        const SensorTime st(ou.obs->sensorTime()); 
        KvalobsDataPtr obs = boost::static_pointer_cast<KvalobsData>(find(st));
        if (not obs)
            obs = boost::static_pointer_cast<KvalobsData>(create(st));
        bool changed = false;
        kvalobs::kvData& d = obs->data();
        if (not Helpers::float_eq()(d.corrected(), ou.corrected)) {
            changed = true;
            d.corrected(ou.corrected);
        }
        if (d.controlinfo() != ou.controlinfo) {
            changed = true;
            d.controlinfo(ou.controlinfo);
        }
        if (changed)
            obsDataChanged(MODIFIED, obs);
    }
    return true;
}

KvalobsDataPtr KvBufferedAccess::receive(const kvalobs::kvData& data)
{
    const SensorTime st(Helpers::sensorTimeFromKvData(data));
    if (not isSubscribed(st))
        return KvalobsDataPtr();
    
    KvalobsDataPtr obs;
    Data_t::iterator it = mData.find(st);
    if (it == mData.end()) {
        obs = boost::make_shared<KvalobsData>(data, false);
        mData[st] = obs;
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
    updateSubscribedTimes();
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
        updateSubscribedTimes();
    }
}

void KvBufferedAccess::updateSubscribedTimes()
{
    mSubscribedTimes.clear();
    BOOST_FOREACH(const ObsSubscription& sub, mSubscriptions) {
        SubscribedTimes_t::iterator it = mSubscribedTimes.find(sub.stationId());
        if (it != mSubscribedTimes.end()) {
            // FIXME merge only overlapping time spans
            TimeRange r(std::min(sub.time().t0(), it->second.t0()), std::max(sub.time().t1(), it->second.t1()));
            it->second = r;
        } else {
            mSubscribedTimes.insert(SubscribedTimes_t::value_type(sub.stationId(), sub.time()));
        }
    }

    std::list<SensorTime> noLongerSubscribed;
    BOOST_FOREACH(const Data_t::value_type d, mData) {
        const SensorTime& dst = d.first;
        if (not isSubscribed(dst))
            noLongerSubscribed.push_back(dst);
    }
    BOOST_FOREACH(const SensorTime& st, noLongerSubscribed) {
        drop(st);
    }
}

bool KvBufferedAccess::isSubscribed(const SensorTime& st)
{
    SubscribedTimes_t::iterator it = mSubscribedTimes.find(st.sensor.stationId);
    if (it == mSubscribedTimes.end())
        return false;
    else
        return it->second.contains(st.time);
}
