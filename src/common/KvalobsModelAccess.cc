
#include "KvalobsModelAccess.hh"

#include "KvServiceHelper.hh"

#include <kvcpp/KvGetDataReceiver.h>
#include <kvcpp/WhichDataHelper.h>

#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.KvalobsModelAccess"
#include "common/ObsLogging.hh"

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

struct sensorString {
  const std::vector<Sensor>& mSensors;
  sensorString(const std::vector<Sensor>& sensors) : mSensors(sensors) { }
};
std::ostream& operator<<(std::ostream& out, const sensorString& sst)
{
  BOOST_FOREACH(const Sensor& s, sst.mSensors)
      out << s;
  return out;
}
} // namespace anonymous

ModelAccess::ModelDataSet KvalobsModelAccess::findMany(const std::vector<SensorTime>& sensorTimes)
{
  METLIBS_LOG_SCOPE();

  kvservice::WhichDataHelper whichData;
  bool empty = true;
  BOOST_FOREACH(const SensorTime& st, sensorTimes) {
    if (not isFetched(st.sensor.stationId, st.time)) {
      whichData.addStation(st.sensor.stationId, timeutil::to_miTime(st.time), timeutil::to_miTime(st.time));
      empty = false;
    }
  }

  if (not empty) {
    try {
      std::list<kvalobs::kvModelData> model;
      if (KvServiceHelper::instance()->getKvModelData(model, whichData)) {
        BOOST_FOREACH(const SensorTime& st, sensorTimes) {
          addFetched(st.sensor.stationId, TimeRange(st.time, st.time));
        }
        BOOST_FOREACH(const kvalobs::kvModelData& md, model) {
          receive(md);
        }
      } else {
        HQC_LOG_ERROR("problem receiving model data for sensors/times " << sensorTimeString(sensorTimes));
      }
    } catch (std::exception& e) {
      HQC_LOG_ERROR("exception while retrieving model data for sensors/times " << sensorTimeString(sensorTimes)
          << ", exception is: " << e.what());
    } catch (...) {
      HQC_LOG_ERROR("exception while retrieving model data for sensors/times " << sensorTimeString(sensorTimes));
    }
  }
  return KvModelAccess::findMany(sensorTimes);
}

bool KvalobsModelAccess::isFetched(int stationId, const timeutil::ptime& t) const
{
  Fetched_t::const_iterator f = mFetched.find(stationId);
  if (f == mFetched.end())
    return false;

  return boost::icl::contains(f->second, t);
}

void KvalobsModelAccess::addFetched(int stationId, const TimeRange& limits)
{
  METLIBS_LOG_SCOPE();
  mFetched[stationId] += boost::icl::interval<timeutil::ptime>::closed(limits.t0(), limits.t1());
  METLIBS_LOG_DEBUG(LOGVAL(stationId) << LOGVAL(mFetched[stationId]));
}

void KvalobsModelAccess::removeFetched(int stationId, const timeutil::ptime& t)
{
  METLIBS_LOG_SCOPE();
  mFetched[stationId] -= t; //boost::icl::interval<timeutil::ptime>::type(limits.t0(), limits.t1());
  METLIBS_LOG_DEBUG(LOGVAL(mFetched[stationId]));
}

ModelAccess::ModelDataSet KvalobsModelAccess::allData(const std::vector<Sensor>& sensors, const TimeRange& limits)
{
  METLIBS_LOG_SCOPE();
  if (not limits.closed()) {
    HQC_LOG_ERROR("invalid time: " << LOGVAL(limits.t0()) << LOGVAL(limits.t1()));
    return ModelDataSet();
  }

  kvservice::WhichDataHelper whichData;
  const FetchedTimes_t limits_set(boost::icl::interval<timeutil::ptime>::closed(limits.t0(), limits.t1()));
  bool empty = true;
  BOOST_FOREACH(const Sensor& s, sensors) {
    if (not s.valid()) {
      HQC_LOG_ERROR("invalid sensor: " << s);
      continue;
    }
    Fetched_t::const_iterator f = mFetched.find(s.stationId);
    if (f == mFetched.end()) {
      METLIBS_LOG_DEBUG("request for station " << s.stationId << " time " << limits);
      whichData.addStation(s.stationId, timeutil::to_miTime(limits.t0()), timeutil::to_miTime(limits.t1()));
      empty = false;
    } else {
      FetchedTimes_t fetched_in_limits;
      boost::icl::add_intersection(fetched_in_limits, limits_set, f->second);
      const FetchedTimes_t to_fetch = limits_set - fetched_in_limits;
      BOOST_FOREACH(const FetchedTimes_t::value_type& t, to_fetch) {
        METLIBS_LOG_DEBUG("request for station " << s.stationId << " time interval " << TimeRange(t.lower(), t.upper()));
        whichData.addStation(s.stationId, timeutil::to_miTime(t.lower()), timeutil::to_miTime(t.upper()));
        empty = false;
      }
    }
  }
  if (not empty) {
    try {
      std::list<kvalobs::kvModelData> model;
      if (KvServiceHelper::instance()->getKvModelData(model, whichData)) {
        BOOST_FOREACH(const Sensor& s, sensors) {
          addFetched(s.stationId, limits);
        }
        BOOST_FOREACH(const kvalobs::kvModelData& md, model) {
          receive(md);
        }
      } else {
        HQC_LOG_ERROR("problem retrieving data for sensors " << sensorString(sensors)
            << " and time " << limits);
      }
    } catch (std::exception& e) {
      HQC_LOG_ERROR("exception while retrieving data for sensors " << sensorString(sensors)
          << " and time " << limits << ", message is: " << e.what());
    } catch (...) {
      HQC_LOG_ERROR("exception while retrieving data for sensors " << sensorString(sensors)
          << " and time " << limits);
    }
  }

  return KvModelAccess::allData(sensors, limits);
}

