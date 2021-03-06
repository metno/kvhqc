
#include "KvalobsAccess.hh"

#include "Functors.hh"
#include "KvHelpers.hh"
#include "KvalobsData.hh"
#include "KvServiceHelper.hh"

#include <kvcpp/KvGetDataReceiver.h>
#include <kvcpp/WhichDataHelper.h>

#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.KvalobsAccess"
#define M_TIME
#include "common/ObsLogging.hh"

namespace kvalobsdata_helpers {

class GetData : public kvservice::KvGetDataReceiver
{
public:
  GetData(KvalobsAccess& ka)
    : mKA(ka) { }

  bool next(kvservice::KvObsDataList &datalist)
    { mKA.nextData(datalist, false); return true; }

private:
  KvalobsAccess& mKA;
};

} /* namespace kvalobsdata_helpers */

// ========================================================================

KvalobsAccess::KvalobsAccess()
  : mLastFetchedStationId(-1)
  , mLastFetchedObsHour(-1)
{
}

KvalobsAccess::~KvalobsAccess()
{
}

ObsAccess::TimeSet KvalobsAccess::allTimes(const std::vector<Sensor>& sensors, const TimeRange& limits)
{
  METLIBS_LOG_TIME();
  findRange(sensors, limits);
  return KvBufferedAccess::allTimes(sensors, limits);
}

ObsAccess::DataSet KvalobsAccess::allData(const std::vector<Sensor>& sensors, const TimeRange& limits)
{
  METLIBS_LOG_TIME();
  findRange(sensors, limits);
  return KvBufferedAccess::allData(sensors, limits);
}

ObsDataPtr KvalobsAccess::find(const SensorTime& st)
{
  METLIBS_LOG_SCOPE();
  ObsDataPtr obs = KvBufferedAccess::find(st);
  if (obs) {
    METLIBS_LOG_DEBUG("found " << st << " in buffer");
    return obs;
  }

  if (isFetched(st.sensor.stationId, st.time)) {
    METLIBS_LOG_DEBUG("fetched " << st << " but not in buffer");
    return KvalobsDataPtr();
  }

  findRange(st.sensor, TimeRange(st.time, st.time));
  obs = KvBufferedAccess::find(st);
  if (not obs)
    METLIBS_LOG_DEBUG("fetch did not put " << st << " in buffer");
  return obs;
}

namespace /* anonymous */ {
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

void KvalobsAccess::findRange(const std::vector<Sensor>& sensors, const TimeRange& limits)
{
  METLIBS_LOG_TIME();
  if (not limits.closed()) {
    HQC_LOG_ERROR("invalid time: " << LOGVAL(limits.t0()) << LOGVAL(limits.t1()));
    return;
  }
  METLIBS_LOG_DEBUG(LOGVAL(limits));

  mCountHoursToFetch = 0;
  mCountFetchedHours = 0;
  kvservice::WhichDataHelper whichData;
  const FetchedTimes_t limits_set(boost::icl::interval<timeutil::ptime>::closed(limits.t0(), limits.t1()));
  std::set<int> stationids;
  BOOST_FOREACH(const Sensor& s, sensors) {
    METLIBS_LOG_DEBUG(LOGVAL(s));
    if (not s.valid()) {
      HQC_LOG_ERROR("invalid sensor: " << s);
      continue;
    }
    Fetched_t::const_iterator f = mFetched.find(s.stationId);
    if (f == mFetched.end()) {
      METLIBS_LOG_DEBUG("request for station " << s.stationId << " time " << limits);
      whichData.addStation(s.stationId, timeutil::to_miTime(limits.t0()), timeutil::to_miTime(limits.t1()));
      stationids.insert(s.stationId);
      mCountHoursToFetch += limits.hours();
    } else {
      FetchedTimes_t fetched_in_limits;
      boost::icl::add_intersection(fetched_in_limits, limits_set, f->second);
      const FetchedTimes_t to_fetch = limits_set - fetched_in_limits;
      if (to_fetch.empty()) METLIBS_LOG_DEBUG("already have all data for station " << s.stationId);
      BOOST_FOREACH(const FetchedTimes_t::value_type& t, to_fetch) {
        const TimeRange interval(t.lower(), t.upper());
        METLIBS_LOG_DEBUG("request for station " << s.stationId << " time interval " << interval);
        whichData.addStation(s.stationId, timeutil::to_miTime(interval.t0()), timeutil::to_miTime(interval.t1()));
        stationids.insert(s.stationId);
        mCountHoursToFetch += interval.hours();
      }
    }
  }
  if (stationids.empty()) {
    METLIBS_LOG_DEBUG("empty for sensors " << sensorString(sensors)
        << " and time " << limits);
    return;
  }

  mLastFetchedStationId = -1;
  mLastFetchedObsHour = -1;
  Q_EMIT signalFetchingData(mCountHoursToFetch, 0);

  kvalobsdata_helpers::GetData get(*this);
  try {
    if (KvServiceHelper::instance()->getKvData(get, whichData)) {
      BOOST_FOREACH(const Sensor& s, sensors) {
        if (s.valid())
          addFetched(s.stationId, limits);
      }
    } else {
      HQC_LOG_ERROR("problem receiving data for sensors " << sensorString(sensors)
          << " and time " << limits);
    }
  } catch (std::exception& e) {
    HQC_LOG_ERROR("exception while retrieving data for sensors " << sensorString(sensors)
        << " and time " << limits << ", message is: " << e.what());
  } catch (...) {
    HQC_LOG_ERROR("exception while retrieving data for sensors " << sensorString(sensors)
        << " and time " << limits);
  }

  mCountHoursToFetch = 0;
  Q_EMIT signalFetchingData(0, 0);
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
  METLIBS_LOG_SCOPE();
  mFetched[stationId] += boost::icl::interval<timeutil::ptime>::closed(limits.t0(), limits.t1());
  METLIBS_LOG_DEBUG(LOGVAL(stationId) << LOGVAL(mFetched[stationId]));
}

void KvalobsAccess::removeFetched(int stationId, const timeutil::ptime& t)
{
  METLIBS_LOG_SCOPE();
  mFetched[stationId] -= t; //boost::icl::interval<timeutil::ptime>::type(limits.t0(), limits.t1());
  METLIBS_LOG_DEBUG(LOGVAL(mFetched[stationId]));
}

void KvalobsAccess::nextData(kvservice::KvObsDataList &dl, bool update)
{
  METLIBS_LOG_TIME();
  BOOST_FOREACH(kvservice::KvObsData& od, dl) {
    METLIBS_LOG_DEBUG(LOGVAL(od.dataList().size()));
    BOOST_FOREACH(const kvalobs::kvData& kvd, od.dataList()) {
      METLIBS_LOG_DEBUG(LOGVAL(kvd));
      receive(kvd, update);

      const int stationId = kvd.stationID(), hour = kvd.obstime().time_of_day().hours();
      const bool differentS = (stationId != mLastFetchedStationId), differentH = (hour != mLastFetchedObsHour);
      mLastFetchedStationId = stationId;
      mLastFetchedObsHour = hour;
      if ((differentS or differentH) and mCountHoursToFetch > 0) {
        mCountFetchedHours += 1;
        if (mCountFetchedHours > mCountHoursToFetch)
          mCountHoursToFetch = mCountFetchedHours+1;
        Q_EMIT signalFetchingData(mCountHoursToFetch, mCountFetchedHours);
      }
    }
  }
}

bool KvalobsAccess::update(const std::vector<ObsUpdate>& updates)
{
  METLIBS_LOG_SCOPE();
  if (not hasReinserter()) {
    METLIBS_LOG_DEBUG("not authorized");
    return false;
  }

  METLIBS_LOG_DEBUG(updates.size() << " updates");
  if (updates.empty())
    return true;

  if (updatesHaveTasks(updates))
    return false;

  std::list<kvalobs::kvData> store;
  std::list<KvalobsDataPtr> modifiedObs, createdObs;
  const timeutil::ptime tbtime = boost::posix_time::microsec_clock::universal_time();
  BOOST_FOREACH(const ObsUpdate& ou, updates) {
    const SensorTime st(ou.obs->sensorTime());
    KvalobsDataPtr obs = std::static_pointer_cast<KvalobsData>(find(st));
    bool created = false;
    if (not obs) {
      obs = std::static_pointer_cast<KvalobsData>(create(st));
      created = true;
    }
    const bool inserted = obs->isCreated();

    kvalobs::kvData& d = obs->data();
    if (inserted and (ou.corrected == kvalobs::NEW_ROW or d.corrected() != kvalobs::NEW_ROW)) {
      HQC_LOG_WARN("attempt to insert unmodified new row: " << obs->sensorTime());
      continue;
    }
    if (inserted and not d.cfailed().empty()) {
      HQC_LOG_WARN("inserting new row with non-empty cfailed: " << obs->sensorTime());
    }

    if (not Helpers::float_eq()(d.corrected(), ou.corrected))
      d.corrected(ou.corrected);
    if (d.controlinfo() != ou.controlinfo)
      d.controlinfo(ou.controlinfo);
    Helpers::updateUseInfo(d);

    if (inserted) {
      Helpers::updateCfailed(d, "hqc-i");
      // specify tbtime
      d = kvalobs::kvData(d.stationID(), d.obstime(), d.original(),
          d.paramID(), timeutil::to_miTime(tbtime), d.typeID(), d.sensor(), d.level(),
          d.corrected(), d.controlinfo(), d.useinfo(), d.cfailed());
    } else {
      Helpers::updateCfailed(d, "hqc-m");
    }
    store.push_back(d);
    (created ? createdObs : modifiedObs).push_back(obs);
    METLIBS_LOG_DEBUG(LOGVAL(st) << LOGVAL(d) << LOGVAL(d.tbtime()) << LOGVAL(d.cfailed())
        << " ins=" << (inserted ? "y" : "n")
        << " create=" << (created ? "y" : "n"));
    //    << " sub=" << (isSubscribed(Helpers::sensorTimeFromKvData(d)) ? "y" : "n"));
  }
  METLIBS_LOG_DEBUG(store.size() << " to store");

  const bool res = mReinserter->insert(store);
  if (res) {
    BOOST_FOREACH(KvalobsDataPtr obs, modifiedObs)
        obsDataChanged(MODIFIED, obs);
    BOOST_FOREACH(KvalobsDataPtr obs, createdObs)
        obsDataChanged(CREATED, obs);
    return true;
  } else {
    std::ostringstream msg;
    msg << "saving these failed: modified:";
    BOOST_FOREACH(KvalobsDataPtr obs, modifiedObs)
        msg << ' ' << obs->sensorTime();
    msg << "; created:";
    BOOST_FOREACH(KvalobsDataPtr obs, createdObs)
        msg << ' ' << obs->sensorTime();
    HQC_LOG_WARN(msg.str());
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
