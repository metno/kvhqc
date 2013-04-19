
#include "KvalobsModelAccess.hh"

#include <kvcpp/KvApp.h>
#include <kvcpp/KvGetDataReceiver.h>
#include <kvcpp/WhichDataHelper.h>

#include <boost/foreach.hpp>

#define NDEBUG
#include "w2debug.hh"

KvalobsModelAccess::KvalobsModelAccess()
{
}

KvalobsModelAccess::~KvalobsModelAccess()
{
}

ModelDataPtr KvalobsModelAccess::find(const SensorTime& st)
{
    Data_t::iterator it = mData.find(st);
    if (it != mData.end())
        return it->second;

    const Fetched f(st.sensor.stationId, st.time);
    if (mFetched.find(f) != mFetched.end())
        return KvalobsModelDataPtr();

    LOG_SCOPE();
    DBG(DBG1(st.sensor.stationId) << DBG1(st.time));
    
    kvservice::WhichDataHelper whichData;
    whichData.addStation(st.sensor.stationId, timeutil::to_miTime(st.time), timeutil::to_miTime(st.time));

    std::list<kvalobs::kvModelData> model;    
    try {
      if (kvservice::KvApp::kvApp->getKvModelData(model, whichData)) {
        mFetched.insert(f);
        BOOST_FOREACH(const kvalobs::kvModelData& md, model)
            receive(md);
      } else {
        LOG4HQC_ERROR("KvalobsModelAccess", "problem receiving model data");
      }
    } catch (std::exception& e) {
      LOG4HQC_ERROR("KvalobsModelAccess", "exception while retrieving model data: " << e.what());
    }
    return KvModelAccess::find(st);
}
