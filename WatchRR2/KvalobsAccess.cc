
#include "KvalobsAccess.hh"
#include "Helpers.hh"
#include "hqc_utilities.hh"
#include "KvalobsData.hh"

#include <kvcpp/KvGetDataReceiver.h>
#include <kvcpp/WhichDataHelper.h>

#include <boost/foreach.hpp>

#define NDEBUG
#include "w2debug.hh"

namespace kvalobsdata_helpers {

class GetData : public kvservice::KvGetDataReceiver
{
public:
    GetData(KvalobsAccess& ka)
        : mKA(ka) { }

    bool next(kvservice::KvObsDataList &datalist)
        { mKA.nextData(datalist); return true; }

private:
    KvalobsAccess& mKA;
};

} /* namespace kvalobsdata_helpers */

// ========================================================================

KvalobsAccess::KvalobsAccess()
    : mDataReinserter(0)
{
}

KvalobsAccess::~KvalobsAccess()
{
}

ObsAccess::TimeSet KvalobsAccess::allTimes(const Sensor& sensor, const TimeRange& limits)
{
    LOG_SCOPE("KvalobsAccess");
    if (not sensor.valid() or limits.undef()) {
        LOG4SCOPE_ERROR("invalid sensor/time: " << sensor << DBG1(limits.t0()) << DBG1(limits.t1()));
        return ObsAccess::TimeSet();
    }
    LOG4SCOPE_DEBUG("allTimes" << DBG1(sensor) << DBG1(limits.t0()) << DBG1(limits.t1()));

    Fetched_t::const_iterator f = mFetched.find(sensor.stationId);
    if (f == mFetched.end()) {
        findRange(sensor, limits);
    } else {
        FetchedTimes_t limits_set, fetched_in_limits;
        limits_set.insert(boost::icl::interval<timeutil::ptime>::closed(limits.t0(), limits.t1()));
        boost::icl::add_intersection(fetched_in_limits, limits_set, f->second);
        FetchedTimes_t to_fetch = limits_set - fetched_in_limits;
        BOOST_FOREACH(const FetchedTimes_t::value_type& t, to_fetch)
            findRange(sensor, TimeRange(t.lower(), t.upper()));
    }
    return KvBufferedAccess::allTimes(sensor, limits);
}

ObsDataPtr KvalobsAccess::find(const SensorTime& st)
{
    ObsDataPtr obs = KvBufferedAccess::find(st);
    if (obs)
        return obs;

    if (isFetched(st.sensor.stationId, st.time))
        return KvalobsDataPtr();

    findRange(st.sensor, TimeRange(st.time, st.time));
    return KvBufferedAccess::find(st);
}

void KvalobsAccess::findRange(const Sensor& sensor, const TimeRange& limits)
{
    LOG_SCOPE("KvalobsAccess");
    if (not sensor.valid() or limits.undef()) {
        LOG4SCOPE_ERROR("invalid sensor/time: " << sensor << DBG1(limits.t0()) << DBG1(limits.t1()));
        return;
    }

    LOG4SCOPE_DEBUG(DBG1(sensor) << DBG1(limits.t0()) << DBG1(limits.t1()));
    
    kvservice::WhichDataHelper whichData;
    whichData.addStation(sensor.stationId, timeutil::to_miTime(limits.t0()), timeutil::to_miTime(limits.t1()));
    
    kvalobsdata_helpers::GetData get(*this);
    try {
      if (kvservice::KvApp::kvApp->getKvData(get, whichData))
        addFetched(sensor.stationId, limits);
     else
       LOG4HQC_ERROR("KvalobsAccess", "problem receiving data");
    } catch (std::exception& e) {
      LOG4HQC_ERROR("KvalobsAccess", "exception while retrieving data: " << e.what());
    }
}

bool KvalobsAccess::isFetched(int stationId, const timeutil::ptime& t) const
{
    Fetched_t::const_iterator f = mFetched.find(stationId);
    if (f == mFetched.end())
        return false;

    return boost::icl::contains(f->second, t);
}

void KvalobsAccess::addFetched(int stationId, const TimeRange& limits)
{
    LOG_SCOPE("KvalobsAccess");
    mFetched[stationId] += boost::icl::interval<timeutil::ptime>::closed(limits.t0(), limits.t1());
    LOG4SCOPE_DEBUG(DBG1(stationId) << DBG1(mFetched[stationId]));
}

void KvalobsAccess::removeFetched(int stationId, const timeutil::ptime& t)
{
    LOG_SCOPE("KvalobsAccess");
    mFetched[stationId] -= t; //boost::icl::interval<timeutil::ptime>::type(limits.t0(), limits.t1());
    LOG4SCOPE_DEBUG(DBG1(mFetched[stationId]));
}

void KvalobsAccess::nextData(kvservice::KvObsDataList &dl)
{
    BOOST_FOREACH(kvservice::KvObsData& od, dl) {
        BOOST_FOREACH(const kvalobs::kvData& kvd, od.dataList())
            receive(kvd);
    }
}

bool KvalobsAccess::update(const std::vector<ObsUpdate>& updates)
{
    LOG_SCOPE("KvalobsAccess");
    if (not hasReinserter()) {
        LOG4SCOPE_DEBUG("not authorized");
        return false;
    }

    LOG4SCOPE_DEBUG(updates.size() << " updates");
    if (updates.empty())
        return true;

    if (updatesHaveTasks(updates))
        return false;

    std::list<kvalobs::kvData> store;
    std::list<KvalobsDataPtr> modifiedObs, createdObs;
    const timeutil::ptime tbtime = boost::posix_time::microsec_clock::universal_time();
    BOOST_FOREACH(const ObsUpdate& ou, updates) {
        const SensorTime st(ou.obs->sensorTime());
        KvalobsDataPtr obs = boost::static_pointer_cast<KvalobsData>(find(st));
        bool created = false;
        if (not obs) {
            obs = boost::static_pointer_cast<KvalobsData>(create(st));
            created = true;
        }
        const bool inserted = obs->isCreated();
        
        kvalobs::kvData& d = obs->data();
        if (not Helpers::float_eq()(d.corrected(), ou.corrected))
            d.corrected(ou.corrected);
        if (d.controlinfo() != ou.controlinfo)
            d.controlinfo(ou.controlinfo);
        Helpers::updateUseInfo(d);

        if (inserted) {
            Helpers::updateCfailed(d, "WatchRR2-i");
            // specify tbtime
            d = kvalobs::kvData(d.stationID(), d.obstime(), d.original(),
                                d.paramID(), timeutil::to_miTime(tbtime), d.typeID(), d.level(), d.sensor(),
                                d.corrected(), d.controlinfo(), d.useinfo(), d.cfailed());
        } else {
            Helpers::updateCfailed(d, "WatchRR2-m");
        }
        store.push_back(d);
        (created ? createdObs : modifiedObs).push_back(obs);
        LOG4SCOPE_DEBUG(DBG1(st) << DBG1(d) << DBG1(d.tbtime()) << DBG1(d.cfailed())
                        << " ins=" << (inserted ? "y" : "n")
                        << " create=" << (created ? "y" : "n"));
        //    << " sub=" << (isSubscribed(Helpers::sensorTimeFromKvData(d)) ? "y" : "n"));
    }
    LOG4SCOPE_DEBUG(store.size() << " to store");

    CKvalObs::CDataSource::Result_var res = mDataReinserter->insert(store);
    if (res->res == CKvalObs::CDataSource::OK) {
        BOOST_FOREACH(KvalobsDataPtr obs, modifiedObs)
            obsDataChanged(MODIFIED, obs);
        BOOST_FOREACH(KvalobsDataPtr obs, createdObs)
            obsDataChanged(CREATED, obs);
        return true;
    } else {
        return false;
    }
}

bool KvalobsAccess::drop(const SensorTime& st)
{
#if 1
#warning implement drop!
    return false;
#else
    const Fetched f(st.sensor.stationId, st.time);
    Fetched_t::iterator fit = mFetched.find(f);
    if (fit != mFetched.end())
        mFetched.erase(fit);
    return KvBufferedAccess::drop(st);
#endif
}
