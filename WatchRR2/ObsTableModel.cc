
#include "ObsTableModel.hh"

#include "ObsAccess.hh"
#include "TimeHeader.hh"

#include <QtGui/QApplication>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

#define NDEBUG
#include "w2debug.hh"

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

void ObsTableModel::addColumn(ObsColumnPtr c)
{
    mColumns.push_back(c);
    if (c)
        c->columnChanged.connect(boost::bind(&ObsTableModel::onColumnChanged, this, _1, _2));
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
    if ((mTimeInRows and orientation == Qt::Horizontal) or ((not mTimeInRows) and orientation == Qt::Vertical)) {
        return columnHeader(section, orientation, role);
    } else if (role == Qt::DisplayRole or role == Qt::ToolTipRole) {
        return TimeHeader::headerData(timeAtRow(section), orientation, role);
    }
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
    for(unsigned int col=0; col<mColumns.size(); ++col) {
        if (mColumns.at(col) and column == mColumns.at(col).get()) {
            const int row = rowAtTime(time);
            if (row >= 0) {
                const QModelIndex index = createIndex(row, col);
                DBG(DBG1(time) << DBG1(col));
                dataChanged(index, index);
            }
            break;
        }
    }
}
