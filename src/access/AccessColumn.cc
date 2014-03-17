
#include "AccessColumn.hh"

#include "common/gui/SensorHeader.hh"

#define MILOGGER_CATEGORY "kvhqc.AccessColumn"
#include "common/ObsLogging.hh"

AccessColumn::AccessColumn(ObsAccess_p da, const Sensor& sensor, const TimeRange& t, DataItemPtr item)
  : mDA(da)
  , mBuffer(new TimeBuffer_p(make_set<Sensor_s>(sensor), t))
  , mItem(item)
  , mHeaderShowStation(true)
{
  TimeBuffer* b = mBuffer.get();
  connect(b, SIGNAL(bufferCompleted(bool)), this, onBufferCompleted(bool));
  connect(b, SIGNAL(newDataEnd(const ObsData_pv&)),    this, SLOT(newDataEnd(const ObsData_pv&)));
  connect(b, SIGNAL(updateDataEnd(const ObsData_pv&)), this, SLOT(updateDataEnd(const ObsData_pv&)));
  connect(b, SIGNAL(dropDataEnd(const SensorTime_v&)), this, SLOT(dropDataEnd(const SensorTime_v&)));
}
      
AccessColumn::~AccessColumn()
{
}

Qt::ItemFlags AccessColumn::flags(const timeutil::ptime& time) const
{
  return mItem->flags(getObs(time));
}
      
QVariant AccessColumn::data(const timeutil::ptime& time, int role) const
{
  const SensorTime st = getSensorTime(time);
  return mItem->data(getObs(time), st, role);
}

bool AccessColumn::setData(const timeutil::ptime&, const QVariant&, int)
{
  return false;
}

QVariant AccessColumn::headerData(Qt::Orientation orientation, int role) const
{
  SensorHeader sh(sensor(), mHeaderShowStation ? SensorHeader::ALWAYS : SensorHeader::TOOLTIP,
      SensorHeader::ALWAYS, mTimeOffset.hours());
  return sh.sensorHeader(mItem, orientation, role);
}
      
void AccessColumn::onBufferCompleted(bool failed)
{
}

void AccessColumn::onNewDataEnd(const ObsData_pv& data)
{
}

void AccessColumn::onUpdateDataEnd(const ObsData_pv& data)
{
}

void AccessColumn::onDropDataEnd(const SensorTime_v& dropped)
{
}

ObsData_b AccessColumn::getObs(const Time& time) const
{
  return mBuffer->get(time);
}

bool AccessColumn::matchSensor(const Sensor& sensorObs) const
{
  return mItem->matchSensor(sensor(), sensorObs);
}
