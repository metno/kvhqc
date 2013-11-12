
#include "NeighborCardsModel.hh"

#include "common/ColumnFactory.hh"
#include "util/Helpers.hh"
#include "common/gui/NeighborHeader.hh"
#include "common/gui/SensorHeader.hh"

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

NeighborCardsModel::NeighborCardsModel(EditAccessPtr da/*, ModelAccessPtr ma*/, const Sensor& sensor, const TimeRange& timeRange)
  : mDA(da)
  , mTimeRange(timeRange)
  , mTime(mTimeRange.t0())
  , mSensors(1, sensor)
{
  Helpers::addNeighbors(mSensors, sensor, mTimeRange, 20);
  mItems.reserve(N_COLUMNS);
  mTimeOffsets.reserve(N_COLUMNS);

  for(int i=0; i<N_COLUMNS; ++i) {
    const Sensor s(sensor.stationId, columnPars[i], sensor.level, sensor.sensor, sensor.typeId);
    mItems.push_back(ColumnFactory::itemForSensor(da, s, columnTypes[i]));
    
    mTimeOffsets.push_back(boost::posix_time::hours(columnTimeOffsets[i]));
  }
  
  mDA->obsDataChanged.connect(boost::bind(&NeighborCardsModel::onDataChanged, this, _1, _2));
}

NeighborCardsModel::~NeighborCardsModel()
{
  mDA->obsDataChanged.disconnect(boost::bind(&NeighborCardsModel::onDataChanged, this, _1, _2));
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
  const EditDataPtr obs = mDA->findE(st);
  return getItem(index)->data(obs, st, role);
}

void NeighborCardsModel::setTime(const timeutil::ptime& time)
{
  if (not mTimeRange.contains(time) or mTime == time)
    return;

  mTime = time;
  /*emit*/ dataChanged(createIndex(0,0), createIndex(mSensors.size()-1, mItems.size()-1));
  /*emit*/ timeChanged(mTime);
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
      return sh.sensorHeader(DataItemPtr(), orientation, role);
    } else {
      return NeighborHeader::headerData(mSensors[0].stationId, mSensors[section].stationId, orientation, role);
    }
  }
  return QVariant();
}

EditDataPtr NeighborCardsModel::getObs(const QModelIndex& index) const
{
  return mDA->findE(getSensorTime(index));
}

SensorTime NeighborCardsModel::getSensorTime(const QModelIndex& index) const
{
  Sensor sensor = mSensors[index.row()];
  sensor.paramId = columnPars[index.column()];
  return SensorTime(sensor, getTime(index));
}

void NeighborCardsModel::onDataChanged(ObsAccess::ObsDataChange what, ObsDataPtr obs)
{
  METLIBS_LOG_SCOPE();
  const SensorTime st(obs->sensorTime());
  METLIBS_LOG_DEBUG(LOGVAL(what) << LOGOBS(obs) << LOGVAL(st.sensor.stationId));

  for(size_t col = 0; col < mTimeOffsets.size(); col += 1) {
    if (st.time == mTime + mTimeOffsets[col]) {
      for(size_t row = 0;row < mItems.size(); row += 1) {
        Sensor sensor = mSensors[row];
        sensor.paramId = columnPars[col];
        if (mItems[row]->matchSensor(sensor, st.sensor)) {
          QModelIndex idx = createIndex(row, col);
          /*emit*/ dataChanged(idx, idx);
        }
      }
    }
  }
}

std::vector<int> NeighborCardsModel::neighborStations() const
{
  std::vector<int> n;
  n.reserve(mSensors.size());
  BOOST_FOREACH(const Sensor& s, mSensors)
      n.push_back(s.stationId);
  return n;
}
