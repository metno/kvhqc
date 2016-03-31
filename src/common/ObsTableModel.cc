
#include "ObsTableModel.hh"

#include "ObsAccess.hh"
#include "common/gui/TimeHeader.hh"

#include <QtGui/QApplication>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.ObsTableModel"
#include "util/HqcLogging.hh"

ObsTableModel::ObsTableModel(EditAccessPtr da, const TimeRange& time, int step)
  : mDA(da)
  , mTimeInRows(true)
  , mTime(time)
  , mTimeStep(step)
{
  METLIBS_LOG_SCOPE();
  METLIBS_LOG_DEBUG(LOGVAL(mTime) << LOGVAL(mTimeStep));
  updateTimes();
}

ObsTableModel::~ObsTableModel()
{
  BOOST_FOREACH(ObsColumnPtr c, mColumns) {
    if (c)
      c->columnChanged.disconnect(boost::bind(&ObsTableModel::onColumnChanged, this, _1, _2));
  }
}

void ObsTableModel::setTimeInRows(bool tir)
{
  if (mTimeInRows == tir)
    return;

  beginResetModel();
  mTimeInRows = tir;
  endResetModel();
  Q_EMIT changedTimeInRows(mTimeInRows);
}

void ObsTableModel::insertColumn(int before, ObsColumnPtr c)
{
  beginResetModel();
  //beginInsertC(before, before);

  mColumns.insert(mColumns.begin() + before, c);
  if (c)
    c->columnChanged.connect(boost::bind(&ObsTableModel::onColumnChanged, this, _1, _2));
  updateTimes();

  //endInsertC();
  endResetModel();
}

void ObsTableModel::moveColumn(int from, int to)
{
  const int cS = mColumns.size();
  if (from == to or (from < 0) or (to < 0) or (from >= cS) or (to >= cS))
    return;

  // do not do remove-insert as this may trigger recalculation of times (usually rows)

  ObsColumnPtr c = getColumn(from);

  beginRemoveC(from, from);
  mColumns.erase(mColumns.begin() + from);
  endRemoveC();

  beginInsertC(to, to);
  mColumns.insert(mColumns.begin() + to, c);
  endInsertC();
}

void ObsTableModel::removeColumn(int at)
{
  beginResetModel();
  //beginRemoveC(at, at);

  const ObsColumns_t::iterator it = mColumns.begin() + at;
  ObsColumnPtr c = *it;
  if (c)
    c->columnChanged.disconnect(boost::bind(&ObsTableModel::onColumnChanged, this, _1, _2));
  mColumns.erase(it);
  updateTimes();

  //endRemoveC();
  endResetModel();
}

void ObsTableModel::setTimeStep(int step)
{
  if (mTimeStep == step)
    return;

  beginResetModel();
  mTimeStep = step;
  updateTimes();
  endResetModel();
  Q_EMIT changedTimeStep(mTimeStep);
}

void ObsTableModel::updateTimes()
{
  METLIBS_LOG_SCOPE();

  mTime0 = mTime.t0();
  if (mTimeStep > 0) {
    const timeutil::ptime tt = mTime0 - boost::posix_time::hours(6);
    const int s = tt.time_of_day().total_seconds();
    const int r = s % mTimeStep;
    mTime0 -= boost::posix_time::seconds(r);

    mRowCount = 1 + (mTime.seconds() / mTimeStep); // integer division
    if (r != 0)
      mRowCount += 1;

    METLIBS_LOG_DEBUG(LOGVAL(mTime0) << LOGVAL(mRowCount) << LOGVAL(mTimeStep) << LOGVAL(r));
  } else {
    mRowCount = 0;
  }
}

int ObsTableModel::rowCount(const QModelIndex&) const
{
  return rowOrColumnCount(true);
}

int ObsTableModel::columnCount(const QModelIndex&) const
{
  return rowOrColumnCount(false);
}

int ObsTableModel::rowOrColumnCount(bool timeDirection) const
{
  if (timeDirection == mTimeInRows) {
    return mRowCount;
  } else {
    return mColumns.size();
  }
}

bool ObsTableModel::isTimeOrientation(Qt::Orientation orientation) const
{
  return (mTimeInRows and orientation == Qt::Vertical)
      or ((not mTimeInRows) and orientation == Qt::Horizontal);
}

void ObsTableModel::beginInsertR(int first, int last)
{
  if (mTimeInRows)
    beginInsertRows(QModelIndex(), first, last);
  else
    beginInsertColumns(QModelIndex(), first, last);
}

void ObsTableModel::beginInsertC(int first, int last)
{
  if (mTimeInRows)
    beginInsertColumns(QModelIndex(), first, last);
  else
    beginInsertRows(QModelIndex(), first, last);
}

void ObsTableModel::endInsertR()
{
  if (mTimeInRows)
    endInsertRows();
  else
    endInsertColumns();
}

void ObsTableModel::endInsertC()
{
  if (mTimeInRows)
    endInsertColumns();
  else
    endInsertRows();
}

void ObsTableModel::beginRemoveR(int first, int last)
{
  if (mTimeInRows)
    beginRemoveRows(QModelIndex(), first, last);
  else
    beginRemoveColumns(QModelIndex(), first, last);
}

void ObsTableModel::beginRemoveC(int first, int last)
{
  if (mTimeInRows)
    beginRemoveColumns(QModelIndex(), first, last);
  else
    beginRemoveRows(QModelIndex(), first, last);
}

void ObsTableModel::endRemoveR()
{
  if (mTimeInRows)
    endRemoveRows();
  else
    endRemoveColumns();
}

void ObsTableModel::endRemoveC()
{
  if (mTimeInRows)
    endRemoveColumns();
  else
    endRemoveRows();
}

Qt::ItemFlags ObsTableModel::flags(const QModelIndex& index) const
{
  ObsColumnPtr oc = getColumn(columnIndex(index));
  if (not oc)
    return 0;
  return oc->flags(timeAtRow(timeIndex(index)));
}

QVariant ObsTableModel::data(const QModelIndex& index, int role) const
{
  ObsColumnPtr oc = getColumn(columnIndex(index));
  if (not oc)
    return QVariant();
  return oc->data(timeAtRow(timeIndex(index)), role);
}

bool ObsTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  ObsColumnPtr oc = getColumn(columnIndex(index));
  if (not oc)
    return false;
  return oc->setData(timeAtRow(timeIndex(index)), value, role);
}

QVariant ObsTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (not isTimeOrientation(orientation))
    return columnHeader(section, orientation, role);

  if (role == Qt::DisplayRole or role == Qt::ToolTipRole)
    return TimeHeader::headerData(timeAtRow(section), orientation, role);
  // if (role == Qt::ForegroundRole) {
  //   const unsigned int wdcolors[7] = { 0x5000C0, 0x900040, 0x900060, 0x900080, 0x9000A0, 0x9000C0, 0x500080 };
  //   const timeutil::ptime t = timeAtRow(section);
  //   return QColor(wdcolors[t.date().day_of_week()]);
  // }
  return QVariant();
}

QVariant ObsTableModel::columnHeader(int section, Qt::Orientation orientation, int role) const
{
  ObsColumnPtr oc = getColumn(section);
  if (oc)
    return oc->headerData(orientation, role);
  else
    return QVariant();
}

timeutil::ptime ObsTableModel::timeAtRow(int row) const
{
  return mTime0 + boost::posix_time::seconds(row * mTimeStep);
}

SensorTime ObsTableModel::findSensorTime(const QModelIndex& idx) const
{
  SensorTime st;
  st.time = timeAtRow(mTimeInRows ? idx.row() : idx.column());

  ObsColumnPtr column = getColumn(mTimeInRows ? idx.column() : idx.row());
  if (column)
    st.sensor = column->sensor();

  return st;
}

int ObsTableModel::rowAtTime(const timeutil::ptime& time) const
{
  METLIBS_LOG_SCOPE();
  if (mTimeStep <= 0)
    return -1;
  const int r = (time - mTime0).total_seconds() / mTimeStep;
  const timeutil::ptime t = timeAtRow(r);
  METLIBS_LOG_DEBUG(LOGVAL(time) << LOGVAL(t) << LOGVAL(r));
  if (t != time)
    return -1;
  else
    return r;
}

void ObsTableModel::onColumnChanged(const timeutil::ptime& time, ObsColumnPtr column)
{
  METLIBS_LOG_SCOPE();

  const ObsColumns_t::const_iterator it = std::find(mColumns.begin(), mColumns.end(), column);
  if (it == mColumns.end())
    return;

  const int row = rowAtTime(time);
  if (row < 0)
    return;
  
  const QModelIndex index = createIndex(row, (it - mColumns.begin()));
  dataChanged(index, index);
}
