
#include "KvalobsModelAccess.hh"

#include "HqcApplication.hh"

#include <kvcpp/KvGetDataReceiver.h>
#include <kvcpp/WhichDataHelper.h>

#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.KvalobsModelAccess"
#include "HqcLogging.hh"

KvalobsModelAccess::KvalobsModelAccess()
{
}

KvalobsModelAccess::~KvalobsModelAccess()
{
}

ModelDataPtr KvalobsModelAccess::find(const SensorTime& st)
{
    ModelDataPtr mdl = KvModelAccess::find(st);
    if (mdl)
        return mdl;

    const Fetched f(st.sensor.stationId, st.time);
    if (mFetched.find(f) != mFetched.end())
        return KvalobsModelDataPtr();

    METLIBS_LOG_SCOPE();
    METLIBS_LOG_DEBUG(LOGVAL(st));
    
    kvservice::WhichDataHelper whichData;
    whichData.addStation(st.sensor.stationId, timeutil::to_miTime(st.time), timeutil::to_miTime(st.time));

    std::list<kvalobs::kvModelData> model;
    try {
      if (hqcApp->getKvModelData(model, whichData)) {
        mFetched.insert(f);
        BOOST_FOREACH(const kvalobs::kvModelData& md, model)
            receive(md);
      } else {
        METLIBS_LOG_ERROR("problem receiving model data for sensor/time " << st);
      }
    } catch (std::exception& e) {
      METLIBS_LOG_ERROR("exception while retrieving model data for sensor/time " << st << ", exception is: " << e.what());
    }
    return KvModelAccess::find(st);
}
