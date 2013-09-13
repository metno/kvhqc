
#include "ObsTableModel.hh"

#include "ObsAccess.hh"
#include "common/gui/TimeHeader.hh"

#include <QtGui/QApplication>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

#define MILOGGER_CATEGORY "kvhqc.ObsTableModel"
#include "util/HqcLogging.hh"

ObsTableModel::ObsTableModel(EditAccessPtr da, const TimeRange& time)
  : mDA(da)
  , mTime(time)
  , mTimeInRows(true)
{
}

ObsTableModel::~ObsTableModel()
{
  BOOST_FOREACH(ObsColumnPtr c, mColumns) {
    if (c)
      c->columnChanged.disconnect(boost::bind(&ObsTableModel::onColumnChanged, this, _1, _2));
  }
}

void ObsTableModel::insertColumn(int before, ObsColumnPtr c)
{
  beginInsertC(before, before);

  mColumns.insert(mColumns.begin() + before, c);
  if (c)
    c->columnChanged.connect(boost::bind(&ObsTableModel::onColumnChanged, this, _1, _2));

  endInsertC();
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
  beginRemoveC(at, at);

  const ObsColumns_t::iterator it = mColumns.begin() + at;
  ObsColumnPtr c = *it;
  if (c)
    c->columnChanged.disconnect(boost::bind(&ObsTableModel::onColumnChanged, this, _1, _2));
  mColumns.erase(it);

  endRemoveC();
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
  if (timeDirection == mTimeInRows)
    return mTime.days() + 1;
  else
    return mColumns.size();
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
  const bool updated = oc->setData(timeAtRow(timeIndex(index)), value, role);
  if (updated)
    dataChanged(index, index);
  return updated;
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
  return mTime.t0() + boost::gregorian::days(row);
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
  const int r = (time - mTime.t0()).hours() / 24;
  if (timeAtRow(r) != time)
    return -1;
  else
    return r;
}

void ObsTableModel::onColumnChanged(const timeutil::ptime& time, ObsColumn* column)
{
  METLIBS_LOG_SCOPE();
  for(unsigned int col=0; col<mColumns.size(); ++col) {
    if (mColumns.at(col) and column == mColumns.at(col).get()) {
      const int row = rowAtTime(time);
      if (row >= 0) {
        const QModelIndex index = createIndex(row, col);
        METLIBS_LOG_DEBUG(LOGVAL(time) << LOGVAL(col));
        dataChanged(index, index);
      }
      break;
    }
  }
}
