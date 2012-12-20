
#include "KvBufferedAccess.hh"
#include "Helpers.hh"

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
    kvalobs::kvData d(s.stationId, timeutil::to_miTime(st.time), kvalobs::MISSING,
                      s.paramId, timeutil::to_miTime(timeutil::ptime()), s.typeId, s.level, s.sensor,
                      kvalobs::NEW_ROW,
                      std::string("0000003000000000"), std::string("0000000000000000"), "");
    KvalobsDataPtr obs = boost::make_shared<KvalobsData>(d);
    mData[st] = obs;
    obsDataChanged(CREATED, obs);
    return obs;
}

bool KvBufferedAccess::update(const std::vector<ObsUpdate>& updates)
{
    LOG_SCOPE();
    // reject anything with tasks
    BOOST_FOREACH(const ObsUpdate& ou, updates) {
        if (ou.tasks != 0) {
            DBG(DBGO1(ou.obs) << " has tasks: " << ou.tasks);
            return false;
        }
    }
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
        DBGO(obs);
        if (changed)
            obsDataChanged(MODIFIED, obs);
    }
    return true;
}

KvalobsDataPtr KvBufferedAccess::receive(const kvalobs::kvData& data)
{
    const SensorTime st(Helpers::sensorTimeFromKvData(data)); 

    KvalobsDataPtr obs;
    Data_t::iterator it = mData.find(st);
    if (it == mData.end()) {
        obs = boost::make_shared<KvalobsData>(data);
        mData[st] = obs;
        obsDataChanged(CREATED, obs);
    } else {
        obs = it->second;

#if 0
        bool changed = false;
        // FIXME must also check for changes in tbtime, original, ...
        kvalobs::kvData& d = obs->data();
        if (not Helpers::float_eq()(d.corrected(), data.corrected())) {
            changed = true;
            d.corrected(data.corrected());
        }
        if (d.controlinfo() != data.controlinfo()) {
            changed = true;
            d.controlinfo(data.controlinfo());
        }
        DBGO(obs);
        if (changed)
            obsDataChanged(MODIFIED, obs);
#else
        // FIXME this might compare too many things ...
        if (obs->data() != data) {
            obs->data() = data;
            obsDataChanged(MODIFIED, obs);
        }
#endif
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
