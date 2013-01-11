
#include "ObsTableModel.hh"

#include "ObsAccess.hh"
#include <QtGui/QApplication>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

#define NDEBUG
#include "w2debug.hh"

namespace /* anonymous */ {
const char* weekdays[7] = {
    QT_TRANSLATE_NOOP("ObsTableModel", "Mon"),
    QT_TRANSLATE_NOOP("ObsTableModel", "Tue"),
    QT_TRANSLATE_NOOP("ObsTableModel", "Wed"),
    QT_TRANSLATE_NOOP("ObsTableModel", "Thu"),
    QT_TRANSLATE_NOOP("ObsTableModel", "Fri"),
    QT_TRANSLATE_NOOP("ObsTableModel", "Sat"),
    QT_TRANSLATE_NOOP("ObsTableModel", "Sun")
};
} // namespace anonymous

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
        ObsColumnPtr oc = getColumn(section);
        if (oc)
            return oc->headerData(role, mTimeInRows);
    } else {
        if (role == Qt::DisplayRole or role == Qt::ToolTipRole) {
            const timeutil::ptime time = timeAtRow(section);
            const boost::gregorian::greg_weekday wd = time.date().day_of_week();
            const QString weekday = qApp->translate("ObsTableModel", weekdays[wd]);
            if (role == Qt::DisplayRole) {
                QString header = "";
                if (orientation == Qt::Vertical)
                    header = weekday + " ";
                header += QString("%1/%2")
                    .arg(time.date().day(), 2)
                    .arg(time.date().month(), 2);
                return header;
            } else if (role == Qt::ToolTipRole) {
                return QString("%1, %2")
                    .arg(weekday)
                    .arg(QString::fromStdString(timeutil::to_iso_extended_string(time)));
            }
        } else if (role == Qt::FontRole) {
            QFont font("Monospace");
            font.setStyleHint(QFont::TypeWriter);
            return font;
        }
    }
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
