
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

KvalobsAccess::KvalobsAccess()
    : mDataReinserter(0)
{
}

KvalobsAccess::~KvalobsAccess()
{
}

ObsDataPtr KvalobsAccess::find(const SensorTime& st)
{
    Data_t::iterator it = mData.find(st);
    if (it != mData.end())
        return it->second;

    const Fetched f(st.sensor.stationId, st.time);
    if (mFetched.find(f) != mFetched.end())
        return KvalobsDataPtr();

    LOG_SCOPE("KvalobsAccess");
    LOG4SCOPE_DEBUG(DBG1(st.sensor.stationId) << DBG1(st.sensor.paramId) << DBG1(st.time));
    
    kvservice::WhichDataHelper whichData;
    whichData.addStation(st.sensor.stationId, timeutil::to_miTime(st.time), timeutil::to_miTime(st.time));
    
    kvalobsdata_helpers::GetData get(*this);
    try {
      if (kvservice::KvApp::kvApp->getKvData(get, whichData))
        mFetched.insert(f);
      else
        LOG4HQC_ERROR("KvalobsAccess", "problem receiving data");
    } catch (std::exception& e) {
      LOG4HQC_ERROR("KvalobsAccess", "exception while retrieving data: " << e.what());
    }
    return KvBufferedAccess::find(st);
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
    if (not mDataReinserter)
        return false;

    LOG4SCOPE_DEBUG(updates.size() << " updates");
    if (updates.empty())
        return true;

    if (updatesHaveTasks(updates))
        return false;

    std::list<kvalobs::kvData> store;
    const timeutil::ptime tbtime = boost::posix_time::microsec_clock::universal_time();
    BOOST_FOREACH(const ObsUpdate& ou, updates) {
        const SensorTime st(ou.obs->sensorTime());
        KvalobsDataPtr obs = boost::static_pointer_cast<KvalobsData>(find(st));
        if (not obs)
            obs = boost::static_pointer_cast<KvalobsData>(create(st));
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
        //DBG(DBG1(d) << DBG1(d.tbtime()) << DBG1(d.cfailed())
        //    << " ins=" << (inserted ? "y" : "n")
        //    << " sub=" << (isSubscribed(Helpers::sensorTimeFromKvData(d)) ? "y" : "n"));
    }
    LOG4SCOPE_DEBUG(store.size() << " to store");

    CKvalObs::CDataSource::Result_var res = mDataReinserter->insert(store);
    if (res->res == CKvalObs::CDataSource::OK) {
        KvBufferedAccess::update(updates);
        return true;
    } else {
        return false;
    }
}

bool KvalobsAccess::drop(const SensorTime& st)
{
    const Fetched f(st.sensor.stationId, st.time);
    Fetched_t::iterator fit = mFetched.find(f);
    if (fit != mFetched.end())
        mFetched.erase(fit);
    return KvBufferedAccess::drop(st);
}
