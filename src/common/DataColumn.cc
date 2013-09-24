
#include "DataColumn.hh"

#include "KvHelpers.hh"
#include "FlagChange.hh"
#include "Tasks.hh"
#include "common/gui/SensorHeader.hh"

#include <kvalobs/kvDataOperations.h>
#include <boost/bind.hpp>
#include <boost/make_shared.hpp>
#include <QtGui/QApplication>
#include <QtGui/QBrush>

#define MILOGGER_CATEGORY "kvhqc.DataColumn"
#include "common/ObsLogging.hh"

DataColumn::DataColumn(EditAccessPtr da, const Sensor& sensor, const TimeRange& t, DataItemPtr item)
  : mDA(da)
  , mSensor(sensor)
  , mTime(t)
  , mItem(item)
  , mHeaderShowStation(true)
{
  mDA->obsDataChanged.connect(boost::bind(&DataColumn::onDataChanged, this, _1, _2));
}

DataColumn::~DataColumn()
{
  mDA->obsDataChanged.disconnect(boost::bind(&DataColumn::onDataChanged, this, _1, _2));
}

Qt::ItemFlags DataColumn::flags(const timeutil::ptime& time) const
{
  return mItem->flags(getObs(time));
}

QVariant DataColumn::data(const timeutil::ptime& time, int role) const
{
  const SensorTime st = getSensorTime(time);
  return mItem->data(getObs(time), st, role);
}

bool DataColumn::setData(const timeutil::ptime& time, const QVariant& value, int role)
{
  METLIBS_LOG_SCOPE();
  try {
    METLIBS_LOG_DEBUG(LOGVAL(value.toString()));
    return mItem->setData(getObs(time), mDA, getSensorTime(time), value, role);
  } catch (std::exception& e) {
    HQC_LOG_WARN(e.what());
    return false;
  }
}

QVariant DataColumn::headerData(Qt::Orientation orientation, int role) const
{
  SensorHeader sh(mSensor, mHeaderShowStation ? SensorHeader::ALWAYS : SensorHeader::TOOLTIP,
      SensorHeader::ALWAYS, mTimeOffset.hours());
  return sh.sensorHeader(mItem, orientation, role);
}

bool DataColumn::onDataChanged(ObsAccess::ObsDataChange what, ObsDataPtr obs)
{
  METLIBS_LOG_SCOPE();
  const SensorTime st(obs->sensorTime());
  if (not mItem->matchSensor(mSensor, st.sensor))
    return false;
  
  METLIBS_LOG_DEBUG(LOGVAL(what) << LOGOBS(obs));
  const timeutil::ptime timeo = st.time - mTimeOffset;
  ObsCache_t::iterator it = mObsCache.find(timeo);
  if (it == mObsCache.end()) {
    METLIBS_LOG_DEBUG("not in cache");
    return false;
  }
  
  mObsCache.erase(it);
  columnChanged(timeo, this);
  return true;
}

EditDataPtr DataColumn::getObs(const timeutil::ptime& time) const
{
  ObsCache_t::iterator it = mObsCache.find(time);
  EditDataPtr obs;
  if (it == mObsCache.end()) {
    obs = mDA->findE(getSensorTime(time));
    mObsCache[time] = obs;
  } else {
    obs = it->second;
  }
  return obs;
}

void DataColumn::setTimeRange(const TimeRange& tr)
{
  mTime = tr.shifted(mTimeOffset);
}

void DataColumn::setTimeOffset(const boost::posix_time::time_duration& timeOffset)
{
  mTime.shift(-mTimeOffset);
  mTimeOffset = timeOffset;
  mTime.shift(mTimeOffset);
}

Sensor DataColumn::sensor() const
{
  return mSensor;
}

bool DataColumn::matchSensor(const Sensor& sensorObs) const
{
  return mItem->matchSensor(mSensor, sensorObs);
}
