
#include "DataListModel.hh"

#include "DataColumn.hh"

#define NDEBUG
#include "debug.hh"

DataListModel::DataListModel(EditAccessPtr eda, const TimeRange& limits)
    : ObsTableModel(eda, limits)
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

            if (mTimeInRows)
                beginInsertRows(index(t, 0), t, t+aE-a);
            else
                beginInsertColumns(index(0, t), t, t+aE-a);
            mTimes.insert(mTimes.begin() + t, addedTimes.begin() + a, addedTimes.begin() + aE);
            if (mTimeInRows)
                endInsertRows();
            else
                endInsertColumns();
            a = aE;
        }
        if (a < addedTimes.size()) {
            const unsigned int tS = mTimes.size();
            if (mTimeInRows)
                beginInsertRows(index(tS, 0), tS, tS+aS-a);
            else
                beginInsertColumns(index(0, tS), tS, tS+aS-a);
            mTimes.insert(mTimes.end(), addedTimes.begin() + a, addedTimes.end());
            if (mTimeInRows)
                endInsertRows();
            else
                endInsertColumns();
        }
    }
}

timeutil::ptime DataListModel::timeAtRow(int row) const
{
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
    Times_t::const_iterator it = std::lower_bound(mTimes.begin(), mTimes.end(), time);
    if (it == mTimes.end() or *it != time)
        return -1;
    return (it - mTimes.begin());
}

int DataListModel::rowOrColumnCount(bool timeDirection) const
{
    if (timeDirection == mTimeInRows)
        return mTimes.size();
    else
        return ObsTableModel::rowOrColumnCount(timeDirection);
}

QVariant DataListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((mTimeInRows and orientation == Qt::Vertical) or ((not mTimeInRows) and orientation == Qt::Horizontal)) {
        if (role == Qt::DisplayRole)
            return QString::fromStdString(timeutil::to_iso_extended_string(timeAtRow(section)));
    }
    return ObsTableModel::headerData(section, orientation, role);
}

