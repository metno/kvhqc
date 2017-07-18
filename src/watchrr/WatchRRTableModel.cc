
#include "WatchRRTableModel.hh"

#if 0
#include "common/ObsAccess.hh"
#include "common/TimeHeader.hh"

#include <QApplication>

#define MILOGGER_CATEGORY "kvhqc.WatchRRTableModel"
#include "common/ObsLogging.hh"

ObsTableModel::ObsTableModel(EditAccess_p da, int step, QObject* parent)
  : QAbstractTableModel(parent)
  , mDA(da)
  , mTimeInRows(true)
  , mTimeStep(step)
  , mTimeCount(0)
  , mHaveBusyColumns(false)
{
  METLIBS_LOG_SCOPE();
}

ObsTableModel::~ObsTableModel()
{
  for (ObsColumn_pv::iterator it = mColumns.begin(); it != mColumns.end(); ++it)
    detachColumn(*it);
}

void ObsTableModel::setTimeSpan(const TimeSpan& limits)
{
  if (limits != mTime) {
    mTime = limits;
    updateTimes();
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

void ObsTableModel::insertColumn(int before, ObsColumn_p c)
{
  columnInsertBegin(before, before);
  mColumns.insert(mColumns.begin() + before, c);
  if (c) {
    connect(c.get(), SIGNAL(columnChanged(const timeutil::ptime&, ObsColumn_p)),
        this, SLOT(onColumnChanged(const timeutil::ptime&, ObsColumn_p)));
    connect(c.get(), SIGNAL(columnTimesChanged(ObsColumn_p)),
        this, SLOT(onColumnTimesChanged(ObsColumn_p)));
    connect(c.get(), SIGNAL(columnBusyStatus(int)),
        this, SLOT(onColumnBusyStatus(int)));
    c->attach(this);
  }
  columnInsertEnd();

  countBusyColumns(true);

  if (c)
    onColumnTimesChanged(c);
}

void ObsTableModel::moveColumn(int from, int to)
{
  const int cS = mColumns.size();
  if (from == to or (from < 0) or (to < 0) or (from >= cS) or (to >= cS))
    return;

  // do not do remove-insert as this may trigger recalculation of times (usually rows)

  ObsColumn_p c = getColumn(from);

  columnRemoveBegin(from, from);
  mColumns.erase(mColumns.begin() + from);
  columnRemoveEnd();

  columnInsertBegin(to, to);
  mColumns.insert(mColumns.begin() + to, c);
  columnInsertEnd();

  if (c)
    onColumnTimesChanged(c);
}

void ObsTableModel::removeColumn(int at)
{
  columnRemoveBegin(at, at);
  const ObsColumn_pv::iterator it = mColumns.begin() + at;
  ObsColumn_p c = *it; // copy pointer, needed after erase / iterator invalidation
  detachColumn(c);
  mColumns.erase(it);
  columnRemoveEnd();

  countBusyColumns(false);

  if (c)
    onColumnTimesChanged(c);
}

void ObsTableModel::detachColumn(ObsColumn_p c)
{
  if (c) {
    c->detach(this);
    disconnect(c.get(), SIGNAL(columnChanged(const timeutil::ptime&, ObsColumn_p)),
        this, SLOT(onColumnChanged(const timeutil::ptime&, ObsColumn_p)));
    disconnect(c.get(), SIGNAL(columnTimesChanged(ObsColumn_p)),
        this, SLOT(onColumnTimesChanged(ObsColumn_p)));
    disconnect(c.get(), SIGNAL(columnBusyStatus(int)),
        this, SLOT(onColumnBusyStatus(int)));
  }
}

void ObsTableModel::removeAllColumns()
{
  beginResetModel();
  for (ObsColumn_pv::iterator it = mColumns.begin(); it != mColumns.end(); ++it) {
    detachColumn(*it);
  }
  mColumns.clear();
  updateTimes();
  endResetModel();

  countBusyColumns(true);
  onColumnTimesChanged(ObsColumn_p());
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
  METLIBS_LOG_SCOPE(LOGMYTYPE() << LOGVAL(mTime) << LOGVAL(mTimeStep));

  if (mTime.closed() and mTimeStep > 0) {
    mTime0 = mTime.t0();
    const timeutil::ptime tt = mTime0 - boost::posix_time::hours(6);
    const int s = tt.time_of_day().total_seconds();
    const int r = s % mTimeStep;
    mTime0 -= boost::posix_time::seconds(r);

    mTimeCount = 1 + (mTime.seconds() / mTimeStep); // integer division
    if (r != 0)
      mTimeCount += 1;

    METLIBS_LOG_DEBUG(LOGVAL(mTime0) << LOGVAL(mTimeCount) << LOGVAL(mTimeStep) << LOGVAL(r));
  } else {
    mTimeCount = 0;
  }
}

ObsColumn_p ObsTableModel::getColumn(int idx) const
{
  if (idx >= 0 and idx < (int)mColumns.size())
    return mColumns[idx];
  return ObsColumn_p();
}

int ObsTableModel::rowCount(const QModelIndex&) const
{
  return mTimeInRows ? countTimes() : countColumns();
}

int ObsTableModel::columnCount(const QModelIndex&) const
{
  return mTimeInRows ? countColumns() : countTimes();
}

int ObsTableModel::countTimes() const
{
  return mTimeCount;
}

int ObsTableModel::countColumns() const
{
  return mColumns.size();
}

bool ObsTableModel::isTimeOrientation(Qt::Orientation orientation) const
{
  return (mTimeInRows and orientation == Qt::Vertical)
      or ((not mTimeInRows) and orientation == Qt::Horizontal);
}

void ObsTableModel::timeInsertBegin(int first, int last)
{
  if (mTimeInRows)
    beginInsertRows(QModelIndex(), first, last);
  else
    beginInsertColumns(QModelIndex(), first, last);
}

void ObsTableModel::timeInsertEnd()
{
  if (mTimeInRows)
    endInsertRows();
  else
    endInsertColumns();
}

void ObsTableModel::columnInsertBegin(int first, int last)
{
  if (mTimeInRows)
    beginInsertColumns(QModelIndex(), first, last);
  else
    beginInsertRows(QModelIndex(), first, last);
}

void ObsTableModel::columnInsertEnd()
{
  if (mTimeInRows)
    endInsertColumns();
  else
    endInsertRows();
}

void ObsTableModel::timeRemoveBegin(int first, int last)
{
  if (mTimeInRows)
    beginRemoveRows(QModelIndex(), first, last);
  else
    beginRemoveColumns(QModelIndex(), first, last);
}

void ObsTableModel::timeRemoveEnd()
{
  if (mTimeInRows)
    endRemoveRows();
  else
    endRemoveColumns();
}

void ObsTableModel::columnRemoveBegin(int first, int last)
{
  if (mTimeInRows)
    beginRemoveColumns(QModelIndex(), first, last);
  else
    beginRemoveRows(QModelIndex(), first, last);
}

void ObsTableModel::columnRemoveEnd()
{
  if (mTimeInRows)
    endRemoveColumns();
  else
    endRemoveRows();
}

Qt::ItemFlags ObsTableModel::flags(const QModelIndex& index) const
{
  if (ObsColumn_p oc = getColumn(columnIndex(index)))
    return oc->flags(timeAtRow(timeIndex(index)));
  return 0;
}

QVariant ObsTableModel::data(const QModelIndex& index, int role) const
{
  if (ObsColumn_p oc = getColumn(columnIndex(index)))
    return oc->data(timeAtRow(timeIndex(index)), role);
  return QVariant();
}

bool ObsTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if (ObsColumn_p oc = getColumn(columnIndex(index)))
    return oc->setData(timeAtRow(timeIndex(index)), value, role);
  return false;
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
  if (ObsColumn_p oc = getColumn(section))
    return oc->headerData(orientation, role);
  return QVariant();
}

timeutil::ptime ObsTableModel::timeAtRow(int row) const
{
  return mTime0 + boost::posix_time::seconds(row * mTimeStep);
}

SensorTime ObsTableModel::findSensorTime(const QModelIndex& idx) const
{
  SensorTime st;
  st.time = timeAtRow(timeIndex(idx));
  if (ObsColumn_p column = getColumn(columnIndex(idx)))
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

void ObsTableModel::onColumnChanged(const timeutil::ptime& time, ObsColumn_p column)
{
  METLIBS_LOG_SCOPE();

  const ObsColumn_pv::const_iterator it = std::find(mColumns.begin(), mColumns.end(), column);
  if (it == mColumns.end())
    return;

  const int row = rowAtTime(time);
  if (row < 0)
    return;
  
  const QModelIndex index = createIndex(row, (it - mColumns.begin()));
  dataChanged(index, index);
}

void ObsTableModel::onColumnTimesChanged(ObsColumn_p column)
{
  METLIBS_LOG_SCOPE(LOGMYTYPE());
  if (column)
    METLIBS_LOG_DEBUG(LOGVAL(column->sensor()));
  beginResetModel();
  updateTimes();
  endResetModel();
}

void ObsTableModel::onColumnBusyStatus(bool busy)
{
  METLIBS_LOG_SCOPE(LOGVAL(busy));
  if (busy == mHaveBusyColumns)
    return;

  countBusyColumns(false);
}

void ObsTableModel::countBusyColumns(bool send)
{
  METLIBS_LOG_SCOPE();
  bool haveBusy = false;
  for (ObsColumn_pv::iterator it = mColumns.begin(); it != mColumns.end(); ++it) {
    if (ObsColumn_p c = *it) {
      if (c->isBusy()) {
        haveBusy = true;
        break;
      }
    }
  }

  if (send or haveBusy != mHaveBusyColumns) {
    mHaveBusyColumns = haveBusy;
    Q_EMIT busyStatus(mHaveBusyColumns);
  }
}
#endif
