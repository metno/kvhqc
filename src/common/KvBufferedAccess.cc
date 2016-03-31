
#include "KvBufferedAccess.hh"

#include "KvHelpers.hh"
#include "KvalobsData.hh"

#include <kvalobs/kvDataOperations.h>

#include <boost/foreach.hpp>
#include <memory>

#define MILOGGER_CATEGORY "kvhqc.KvBufferedAccess"
#include "common/ObsLogging.hh"

ObsAccess::TimeSet KvBufferedAccess::allTimes(const std::vector<Sensor>& sensors, const TimeRange& limits)
{
  TimeSet times;

  BOOST_FOREACH(const Sensor& sensor, sensors) {
    for (Data_t::const_iterator it = mData.lower_bound(SensorTime(sensor, limits.t0())); it != mData.end(); ++it)
    {
      const SensorTime& dst = it->first;
      if ((not eq_Sensor()(dst.sensor, sensor)) or (dst.time > limits.t1()))
        break;
      if (it->second)
        times.insert(dst.time);
    }
  }
  return times;
}

ObsAccess::DataSet KvBufferedAccess::allData(const std::vector<Sensor>& sensors, const TimeRange& limits)
{
  DataSet data;

  BOOST_FOREACH(const Sensor& sensor, sensors) {
    for (Data_t::const_iterator it = mData.lower_bound(SensorTime(sensor, limits.t0())); it != mData.end(); ++it) {
      const SensorTime& dst = it->first;
      if ((not eq_Sensor()(dst.sensor, sensor)) or (dst.time > limits.t1()))
        break;
      if (it->second)
        data.insert(it->second);
    }
  }
  return data;
}

ObsDataPtr KvBufferedAccess::find(const SensorTime& st)
{
  if (not st.valid()) {
    HQC_LOG_ERROR("invalid sensorTime: " << st);
    return ObsDataPtr();
  }

  Data_t::iterator it = mData.find(st);
  if (it != mData.end())
    return it->second;

  return KvalobsDataPtr();
}

ObsDataPtr KvBufferedAccess::create(const SensorTime& st)
{
  if (not st.valid())
    HQC_LOG_ERROR("invalid sensorTime: " << st);

  Data_t::iterator it = mData.find(st);
  if (it != mData.end() and it->second)
    return it->second;

  const Sensor& s = st.sensor;
  kvalobs::kvData d = kvalobs::getMissingKvData(s.stationId, timeutil::to_miTime(st.time),
      s.paramId, s.typeId, s.sensor, s.level);
  d.corrected(kvalobs::NEW_ROW);
  KvalobsDataPtr obs = std::make_shared<KvalobsData>(d, true);
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

void KvBufferedAccess::receive(const kvalobs::kvData& data, bool update)
{
  const SensorTime st(Helpers::sensorTimeFromKvData(data));
  // try inserting a null pointer
  const std::pair<Data_t::iterator, bool> ins = mData.insert(std::make_pair(st, KvalobsDataPtr()));
  if (ins.second) {
    // actually inserted -> replace null with data
    KvalobsDataPtr obs = std::make_shared<KvalobsData>(data, false);
    ins.first->second = obs;
    if (update)
      obsDataChanged(CREATED, obs);
  } else {
    // already present -> check for change
    KvalobsDataPtr obs = ins.first->second;
    const bool hadBeenCreated = obs->isCreated();
    obs->setCreated(false);

    // FIXME this might compare too many things ...
    const bool hasChanged = not kvalobs::compare::exactly_equal()(obs->data(), data);
    if (hasChanged)
      obs->data() = data;
    if (hadBeenCreated or hasChanged)
      obsDataChanged(MODIFIED, obs);
  }
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
