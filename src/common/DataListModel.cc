
#include "DataListModel.hh"

#include "DataColumn.hh"
#include "KvMetaDataBuffer.hh"
#include "util/Helpers.hh"

#include <QtGui/QBrush>

#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.DataListModel"
#include "util/HqcLogging.hh"

DataListModel::DataListModel(EditAccessPtr eda, const TimeRange& limits)
  : ObsTableModel(eda, limits, 0)
  , mFilterByTimestep(true)
  , mCenter(0)
{
  updateTimes();
}

DataListModel::~DataListModel()
{
}

void DataListModel::setFilterByTimestep(bool fbts)
{
  if (fbts != mFilterByTimestep) {
    mFilterByTimestep = fbts;
    beginResetModel();
    updateTimes();
    endResetModel();
  }
}  

void DataListModel::updateTimes()
{
  METLIBS_LOG_SCOPE();
  ObsTableModel::updateTimes();

  ObsAccess::TimeSet oldTimes(mTimes.begin(), mTimes.end()), newTimes;
  for (int i=0; i<columnCount(QModelIndex()); ++i) {
    ObsColumnPtr c = getColumn(i);
    if (c)
      mDA->addAllTimes(newTimes, c->sensor(), mTime);
  }

  mTimes = Times_t(newTimes.begin(), newTimes.end());

  if (getTimeStep() > 0 and mFilterByTimestep) {
    mTimesFiltered.clear();
    ObsAccess::TimeSet filterTimes;
    BOOST_FOREACH(const timeutil::ptime& t, newTimes) {
      const int s = (t - mTime0).total_seconds();
      METLIBS_LOG_DEBUG(LOGVAL(t) << LOGVAL(mTime0) << LOGVAL(s) << LOGVAL(mTimeStep));
      if ((s % mTimeStep) == 0)
        mTimesFiltered.push_back(t);
    }
  } else {
    mTimesFiltered = mTimes;
  }
}

timeutil::ptime DataListModel::timeAtRow(int row) const
{
  if (getTimeStep() > 0 and not mFilterByTimestep)
    return ObsTableModel::timeAtRow(row);
  return mTimesFiltered.at(row);
}

QModelIndexList DataListModel::findIndexes(const SensorTime& st)
{
  // FIXME this does not take into account columns with time offset
  QModelIndexList idxs;
  const int row = rowAtTime(st.time);
  if (row >= 0) {
    const int nColumns = columnCount(QModelIndex());
    for (int col=0; col<nColumns; ++col) {
      DataColumnPtr dc = boost::dynamic_pointer_cast<DataColumn>(getColumn(col));
      if (not dc)
        continue;
      if (dc->matchSensor(st.sensor))
        idxs << index(row, col, QModelIndex());
    }
  }
  return idxs;
}

int DataListModel::rowAtTime(const timeutil::ptime& time) const
{
  if (getTimeStep() > 0 and not mFilterByTimestep)
    return ObsTableModel::rowAtTime(time);

  Times_t::const_iterator it = std::lower_bound(mTimesFiltered.begin(), mTimesFiltered.end(), time);
  if (it == mTimes.end() or *it != time)
    return -1;
  return (it - mTimesFiltered.begin());
}

int DataListModel::rowOrColumnCount(bool timeDirection) const
{
  if (timeDirection == mTimeInRows and not (getTimeStep() > 0 and not mFilterByTimestep))
    return mTimesFiltered.size();
  return ObsTableModel::rowOrColumnCount(timeDirection);
}

QVariant DataListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (isTimeOrientation(orientation) and role == Qt::DisplayRole)
    return QString::fromStdString(timeutil::to_iso_extended_string(timeAtRow(section)));

  return ObsTableModel::headerData(section, orientation, role);
}

QVariant DataListModel::columnHeader(int section, Qt::Orientation orientation, int role) const
{
  const QVariant hdr = ObsTableModel::columnHeader(section, orientation, role);
  if (mCenter == 0)
    return hdr;
  if (role == Qt::DisplayRole or role == Qt::ToolTipRole or role == Qt::StatusTipRole or role == Qt::ForegroundRole) {
    ObsColumnPtr oc = getColumn(section);
    if (oc and oc->sensor().stationId != mCenter) {
      METLIBS_LOG_SCOPE();
      try {
        const kvalobs::kvStation& sc = KvMetaDataBuffer::instance()->findStation(mCenter);
        const kvalobs::kvStation& sn = KvMetaDataBuffer::instance()->findStation(oc->sensor().stationId);

        const float distanceKm = Helpers::distance(sc.lon(), sc.lat(), sn.lon(), sn.lat());
        if (role == Qt::ForegroundRole) {
          const float MAX_DIST = 150;
          const float x = std::min(distanceKm, MAX_DIST) / MAX_DIST;
          METLIBS_LOG_DEBUG(LOGVAL(distanceKm) << LOGVAL(x));
          return QColor(int(0xC0 * x), int(0xE0 * (1-x)), int(8*(oc->sensor().stationId % 13)));
        } else {
          const QString sep = (orientation == Qt::Horizontal) ? "\n" : " ";
          QString distance;
          if (role == Qt::DisplayRole)
            distance = "[" + QString::number(distanceKm, 'f', 0) + "km]";
          else
            distance = tr("Distance: %1km").arg(distanceKm, 0, 'f', 0);
          return hdr.toString() + sep + distance;
        }
      } catch (std::exception& e) {
        HQC_LOG_WARN("exception for neighbor header: " << e.what());
      }
    }
  }
  return hdr;
}

