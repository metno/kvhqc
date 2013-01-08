
#include "KvalobsAccess.hh"
#include "Helpers.hh"

#include <kvcpp/KvGetDataReceiver.h>
#include <kvcpp/WhichDataHelper.h>

#include <boost/foreach.hpp>

//#define NDEBUG
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

    LOG_SCOPE();
    DBG(DBG1(st.sensor.stationId) << DBG1(st.time));
    
    kvservice::WhichDataHelper whichData;
    whichData.addStation(st.sensor.stationId, timeutil::to_miTime(st.time), timeutil::to_miTime(st.time));
    
    kvalobsdata_helpers::GetData get(*this);
    if (kvservice::KvApp::kvApp->getKvData(get, whichData))
        mFetched.insert(f);
    else
        std::cerr << "problem receiving data" << std::endl;
    return KvBufferedAccess::find(st);
}

void KvalobsAccess::nextData(kvservice::KvObsDataList &dl)
{
    LOG_SCOPE();
    BOOST_FOREACH(kvservice::KvObsData& od, dl) {
        BOOST_FOREACH(const kvalobs::kvData& kvd, od.dataList())
            receive(kvd);
    }
}

bool KvalobsAccess::update(const std::vector<ObsUpdate>& updates)
{
    LOG_SCOPE();
    if (not mDataReinserter)
        return false;

    if (updates.empty())
        return true;

    if (updatesHaveTasks(updates))
        return false;

    std::list<kvalobs::kvData> store;
    const timeutil::ptime tbtime = timeutil::now();
    BOOST_FOREACH(const ObsUpdate& ou, updates) {
        const SensorTime st(ou.obs->sensorTime());
        bool insert = false;
        KvalobsDataPtr obs = boost::static_pointer_cast<KvalobsData>(find(st));
        if (not obs) {
            insert = true;
            obs = boost::static_pointer_cast<KvalobsData>(create(st));
        }
        
        kvalobs::kvData& d = obs->data();
        if (not Helpers::float_eq()(d.corrected(), ou.corrected))
            d.corrected(ou.corrected);
        if (d.controlinfo() != ou.controlinfo)
            d.controlinfo(ou.controlinfo);
        Helpers::updateUseInfo(d);

        if (insert) {
            Helpers::updateCfailed(d, "WatchRR2-i");
            // specify tbtime
            d = kvalobs::kvData(d.stationID(), d.obstime(), d.original(),
                                d.paramID(), timeutil::to_miTime(tbtime), d.typeID(), d.level(), d.sensor(),
                                d.corrected(), d.controlinfo(), d.useinfo(), d.cfailed());
        } else {
            Helpers::updateCfailed(d, "WatchRR2-m");
        }
        store.push_back(d);
        DBG(DBG1(d) << " sub=" << (isSubscribed(Helpers::sensorTimeFromKvData(d)) ? "y" : "n"));
    }

    CKvalObs::CDataSource::Result_var res = mDataReinserter->insert(store);
    if (res->res == CKvalObs::CDataSource::OK) {
        KvBufferedAccess::update(updates);
        return true;
    } else {
        return false;
    }
}
