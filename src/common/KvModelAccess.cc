
#include "KvModelAccess.hh"

#include "KvHelpers.hh"

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

#define MILOGGER_CATEGORY "kvhqc.KvModelAccess"
#include "common/ObsLogging.hh"

ModelDataPtr KvModelAccess::find(const SensorTime& st)
{
  METLIBS_LOG_SCOPE();
  METLIBS_LOG_DEBUG(LOGVAL(st));

  const ModelDataSet data = findMany(std::vector<SensorTime>(1, st));
  if (data.size() == 1)
    return *data.begin();
  return KvalobsModelDataPtr();
}

ModelAccess::ModelDataSet KvModelAccess::findMany(const std::vector<SensorTime>& sensorTimes)
{
  METLIBS_LOG_SCOPE();

  ModelDataSet data;
  BOOST_FOREACH(const SensorTime& st, sensorTimes) {
    const Data_t::iterator it = mData.find(st);
    if (it != mData.end())
      data.insert(it->second);
  }
  
  return data;
}

ModelAccess::ModelDataSet KvModelAccess::allData(const std::vector<Sensor>& sensors, const TimeSpan& limits)
{
  METLIBS_LOG_SCOPE();

  ModelDataSet data;
  BOOST_FOREACH(const Sensor& s, sensors) {
    for (Data_t::const_iterator it = mData.lower_bound(SensorTime(s, limits.t0())); it != mData.end(); ++it) {
      const SensorTime& dst = it->first;
      if ((not eq_ModelSensor()(dst.sensor, s)) or (dst.time > limits.t1()))
        break;
      if (it->second)
        data.insert(it->second);
    }
  }
  
  return data;
}

KvalobsModelDataPtr KvModelAccess::receive(const kvalobs::kvModelData& data)
{
  METLIBS_LOG_SCOPE();
  const SensorTime st(Helpers::sensorTimeFromKvModelData(data)); 
  METLIBS_LOG_DEBUG(LOGVAL(st) << LOGVAL(data));

  KvalobsModelDataPtr mdl;
  Data_t::iterator it = mData.find(st);
  if (it == mData.end()) {
    mdl = boost::make_shared<KvalobsModelData>(data);
    mData.insert(std::make_pair(st, mdl));
    modelDataChanged(mdl);
    METLIBS_LOG_DEBUG("new model data");
  } else {
    mdl = it->second;

    // TODO this might compare too many things ...
    if (mdl->data() != data) {
      mdl->data() = data;
      modelDataChanged(mdl);
    }
  }
  return mdl;
}

bool KvModelAccess::drop(const SensorTime& st)
{
  Data_t::iterator it = mData.find(st);
  if (it == mData.end())
    return false;

  ModelDataPtr mdl = it->second;
  mData.erase(it);
  modelDataChanged(mdl);
  return true;
}
