
#include "DataColumn.hh"

#include "common/gui/SensorHeader.hh"
#include "util/make_set.hh"

#include <boost/make_shared.hpp>

#define MILOGGER_CATEGORY "kvhqc.DataColumn"
#include "common/ObsLogging.hh"

DataColumn::DataColumn(EditAccess_p da, const Sensor& sensor, const TimeRange& t, DataItem_p item)
  : mDA(da)
  , mBuffer(boost::make_shared<TimeBuffer>(make_set<Sensor_s>(sensor), t))
  , mItem(item)
  , mHeaderShowStation(true)
{
  TimeBuffer* b = mBuffer.get();
  connect(b, SIGNAL(bufferCompleted(bool)),            this, SLOT(onBufferCompleted(bool)));
  connect(b, SIGNAL(newDataEnd(const ObsData_pv&)),    this, SLOT(newDataEnd(const ObsData_pv&)));
  connect(b, SIGNAL(updateDataEnd(const ObsData_pv&)), this, SLOT(updateDataEnd(const ObsData_pv&)));
  connect(b, SIGNAL(dropDataEnd(const SensorTime_v&)), this, SLOT(dropDataEnd(const SensorTime_v&)));
}
      
DataColumn::~DataColumn()
{
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

bool DataColumn::setData(const timeutil::ptime&, const QVariant&, int)
{
  return false;
}

QVariant DataColumn::headerData(Qt::Orientation orientation, int role) const
{
  SensorHeader sh(sensor(), mHeaderShowStation ? SensorHeader::ALWAYS : SensorHeader::TOOLTIP,
      SensorHeader::ALWAYS, mTimeOffset.hours());
  return sh.sensorHeader(mItem, orientation, role);
}
      
void DataColumn::onBufferCompleted(bool failed)
{
}

void DataColumn::onNewDataEnd(const ObsData_pv& data)
{
}

void DataColumn::onUpdateDataEnd(const ObsData_pv& data)
{
}

void DataColumn::onDropDataEnd(const SensorTime_v& dropped)
{
}

ObsData_p DataColumn::getObs(const Time& time) const
{
  return mBuffer->get(getSensorTime(time));
}

bool DataColumn::matchSensor(const Sensor& sensorObs) const
{
  return mItem->matchSensor(sensor(), sensorObs);
}
