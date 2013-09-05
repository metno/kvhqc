
#include "KvalobsModelAccess.hh"

#include "KvServiceHelper.hh"

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

namespace /* anonymous */ {
struct sensorTimeString {
  const std::vector<SensorTime>& mSensorTimes;
  sensorTimeString(const std::vector<SensorTime>& sensorTimes) : mSensorTimes(sensorTimes) { }
};
std::ostream& operator<<(std::ostream& out, const sensorTimeString& stst)
{
  BOOST_FOREACH(const SensorTime& st, stst.mSensorTimes)
      out << st;
  return out;
}
} // namespace anonymous

ModelAccess::ModelDataSet KvalobsModelAccess::findMany(const std::vector<SensorTime>& sensorTimes)
{
  METLIBS_LOG_SCOPE();

  kvservice::WhichDataHelper whichData;
  bool empty = true;
  BOOST_FOREACH(const SensorTime& st, sensorTimes) {
    const Fetched f(st.sensor.stationId, st.time);
    if (mFetched.find(f) == mFetched.end()) {
      whichData.addStation(st.sensor.stationId, timeutil::to_miTime(st.time), timeutil::to_miTime(st.time));
      empty = false;
    }
  }
  
  if (not empty) {
    std::list<kvalobs::kvModelData> model;
    try {
      if (KvServiceHelper::instance()->getKvModelData(model, whichData)) {
        BOOST_FOREACH(const SensorTime& st, sensorTimes) {
          const Fetched f(st.sensor.stationId, st.time);
          mFetched.insert(f);
        }
        BOOST_FOREACH(const kvalobs::kvModelData& md, model) {
          receive(md);
        }
      } else {
        METLIBS_LOG_ERROR("problem receiving model data for sensors/times " << sensorTimeString(sensorTimes));
      }
    } catch (std::exception& e) {
      METLIBS_LOG_ERROR("exception while retrieving model data for sensors/times " << sensorTimeString(sensorTimes)
          << ", exception is: " << e.what());
    } catch (...) {
      METLIBS_LOG_ERROR("exception while retrieving model data for sensors/times " << sensorTimeString(sensorTimes));
    }
  }
  return KvModelAccess::findMany(sensorTimes);
}
