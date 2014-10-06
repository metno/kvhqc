
#include "NeighborCardsModel.hh"

#include "common/ColumnFactory.hh"
#include "common/KvHelpers.hh"
#include "common/NeighborHeader.hh"
#include "common/ObsPgmRequest.hh"
#include "common/SensorHeader.hh"
#include "util/Helpers.hh"

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.NeighborCardsModel"
#include "common/ObsLogging.hh"

namespace /* anonymous */ {
const int N_COLUMNS = 18;
const int columnPars[N_COLUMNS] = {
  kvalobs::PARAMID_V4, kvalobs::PARAMID_V5, kvalobs::PARAMID_V6,
  kvalobs::PARAMID_V4, kvalobs::PARAMID_V5, kvalobs::PARAMID_V6,
  kvalobs::PARAMID_V4, kvalobs::PARAMID_V5, kvalobs::PARAMID_V6,
  kvalobs::PARAMID_RR_24, kvalobs::PARAMID_SA, kvalobs::PARAMID_SD,
  kvalobs::PARAMID_RR_24, kvalobs::PARAMID_SA, kvalobs::PARAMID_SD,
  kvalobs::PARAMID_RR_24, kvalobs::PARAMID_SA, kvalobs::PARAMID_SD
};
const int N_UNIQUE_PARS = 6;
const int uniqueColumnPars[N_UNIQUE_PARS] = {
  kvalobs::PARAMID_V4, kvalobs::PARAMID_V5, kvalobs::PARAMID_V6,
  kvalobs::PARAMID_RR_24, kvalobs::PARAMID_SA, kvalobs::PARAMID_SD
};
const ObsColumn::Type columnTypes[N_COLUMNS] = {
  ObsColumn::NEW_CORRECTED,   ObsColumn::NEW_CORRECTED,   ObsColumn::NEW_CORRECTED,
  ObsColumn::NEW_CORRECTED,   ObsColumn::NEW_CORRECTED,   ObsColumn::NEW_CORRECTED,
  ObsColumn::NEW_CORRECTED,   ObsColumn::NEW_CORRECTED,   ObsColumn::NEW_CORRECTED,
  ObsColumn::NEW_CORRECTED,   ObsColumn::NEW_CORRECTED,   ObsColumn::NEW_CORRECTED,
  ObsColumn::ORIGINAL,        ObsColumn::ORIGINAL,        ObsColumn::ORIGINAL,
  ObsColumn::NEW_CONTROLINFO, ObsColumn::NEW_CONTROLINFO, ObsColumn::NEW_CONTROLINFO
};
const int columnTimeOffsets[N_COLUMNS] = {
  -18, -18, -18,
  -12, -12, -12,
  0, 0, 0,
  0, 0, 0,
  0, 0, 0,
  0, 0, 0
};
} // namespace anonymous

NeighborCardsModel::NeighborCardsModel(TaskAccess_p da/*, ModelAccessPtr ma*/, const Sensor& sensor, const TimeSpan& timeRange)
  : mDA(da)
  , mTimeSpan(timeRange)
  , mTime(mTimeSpan.t0())
  , mSensors(1, sensor)
{
  hqc::int_s stationIds = KvMetaDataBuffer::instance()->findNeighborStationIds(sensor.stationId);
  stationIds.insert(sensor.stationId);

  ObsPgmRequest* mObsPgmRequest = new ObsPgmRequest(stationIds);
  mObsPgmRequest->sync();
  
  KvMetaDataBuffer::instance()->addNeighbors(mSensors, sensor, mTimeSpan, mObsPgmRequest, 20);
  mItems.reserve(N_COLUMNS);
  mTimeOffsets.reserve(N_COLUMNS);

  for(int i=0; i<N_COLUMNS; ++i) {
    const Sensor s(sensor.stationId, columnPars[i], sensor.level, sensor.sensor, sensor.typeId);
    mItems.push_back(ColumnFactory::itemForSensor(da, s, columnTypes[i]));
    
    mTimeOffsets.push_back(boost::posix_time::hours(columnTimeOffsets[i]));
  }

  delete mObsPgmRequest;
}

NeighborCardsModel::~NeighborCardsModel()
{
}

int NeighborCardsModel::rowCount(const QModelIndex&) const
{
  return mSensors.size();
}

int NeighborCardsModel::columnCount(const QModelIndex&) const
{
  return mItems.size();
}

Qt::ItemFlags NeighborCardsModel::flags(const QModelIndex& index) const
{
  return getItem(index)->flags(getObs(index)) & ~Qt::ItemIsEditable;
}

QVariant NeighborCardsModel::data(const QModelIndex& index, int role) const
{
  const SensorTime st(getSensorTime(index));
  const ObsData_p obs = mDA->findE(st);
  return getItem(index)->data(obs, st, role);
}

void NeighborCardsModel::setTime(const timeutil::ptime& time)
{
  if (not mTimeSpan.contains(time) or mTime == time)
    return;

  mTime = time;
  Q_EMIT dataChanged(createIndex(0,0), createIndex(mSensors.size()-1, mItems.size()-1));
  Q_EMIT timeChanged(mTime);
}

QVariant NeighborCardsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole or role == Qt::ToolTipRole) {
    if (orientation == Qt::Horizontal) {
      const Sensor s(-1, columnPars[section], -1, -1, -1);
      SensorHeader sh(s, SensorHeader::NEVER, SensorHeader::ALWAYS, columnTimeOffsets[section]);
      return sh.sensorHeader(mItems[section], orientation, role);
    } else if (role == Qt::ToolTipRole) {
      SensorHeader sh(mSensors[section], SensorHeader::ALWAYS, SensorHeader::NEVER, 0);
      return sh.sensorHeader(DataItem_p(), orientation, role);
    } else {
      return NeighborHeader::headerData(mSensors[0].stationId, mSensors[section].stationId, orientation, role);
    }
  }
  return QVariant();
}

ObsData_p NeighborCardsModel::getObs(const QModelIndex& index) const
{
  return mDA->findE(getSensorTime(index));
}

SensorTime NeighborCardsModel::getSensorTime(const QModelIndex& index) const
{
  Sensor sensor = mSensors[index.row()];
  sensor.paramId = columnPars[index.column()];
  return SensorTime(sensor, getTime(index));
}

std::vector<int> NeighborCardsModel::neighborStations() const
{
  std::vector<int> n;
  n.reserve(mSensors.size());
  BOOST_FOREACH(const Sensor& s, mSensors)
      n.push_back(s.stationId);
  return n;
}
