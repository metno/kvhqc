
#include "KvModelAccess.hh"
#include "Helpers.hh"

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

#define MILOGGER_CATEGORY "kvhqc.KvModelAccess"
#include "HqcLogging.hh"

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
    Data_t::iterator it;
    if (isModelSensorTime(st)) {
      it = mData.find(st);
    } else {
      it = mData.find(makeModelSensorTime(st));
    }
    if (it != mData.end())
      data.insert(it->second);
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
