
#include "DataListModel.hh"

#include "DataColumn.hh"
#include "KvMetaDataBuffer.hh"
#include "util/Helpers.hh"

#include <QtGui/QBrush>

#define MILOGGER_CATEGORY "kvhqc.DataListModel"
#include "util/HqcLogging.hh"

DataListModel::DataListModel(EditAccessPtr eda, const TimeRange& limits)
  : ObsTableModel(eda, limits, 0)
  , mCenter(0)
{
}

DataListModel::~DataListModel()
{
}

void DataListModel::insertColumn(int before, ObsColumnPtr c)
{
  ObsTableModel::insertColumn(before, c);

  ObsAccess::TimeSet oldTimes(mTimes.begin(), mTimes.end()), newTimes(oldTimes);
  mDA->addAllTimes(newTimes, c->sensor(), mTime);

  Times_t addedTimes;
  std::set_difference(newTimes.begin(), newTimes.end(), oldTimes.begin(), oldTimes.end(),
      std::back_inserter<Times_t>(addedTimes));
  if (not addedTimes.empty()) {
    const unsigned int aS = addedTimes.size();
    unsigned int a = 0;
    for (unsigned int t = 0; a < aS and t < mTimes.size(); ++t) {
      if (mTimes[t] <= addedTimes[a])
        continue;
      unsigned int aE = a+1;
      while (aE < aS && mTimes[t] > addedTimes[aE])
        aE += 1;

      beginInsertR(t, t+aE-a-1);
      mTimes.insert(mTimes.begin() + t, addedTimes.begin() + a, addedTimes.begin() + aE);
      endInsertR();
      a = aE;
    }
    if (a < addedTimes.size()) {
      const unsigned int tS = mTimes.size();
      beginInsertR(tS, tS+aS-a-1);
      mTimes.insert(mTimes.end(), addedTimes.begin() + a, addedTimes.end());
      endInsertR();
    }
  }
}

void DataListModel::removeColumn(int at)
{
  ObsTableModel::removeColumn(at);

  // TODO update mTimes
}

timeutil::ptime DataListModel::timeAtRow(int row) const
{
  if (getTimeStep() > 0)
    return ObsTableModel::timeAtRow(row);
  return mTimes.at(row);
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
  if (getTimeStep() > 0)
    return ObsTableModel::rowAtTime(time);

  Times_t::const_iterator it = std::lower_bound(mTimes.begin(), mTimes.end(), time);
  if (it == mTimes.end() or *it != time)
    return -1;
  return (it - mTimes.begin());
}

int DataListModel::rowOrColumnCount(bool timeDirection) const
{
  if (timeDirection == mTimeInRows and getTimeStep() <= 0)
    return mTimes.size();
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

