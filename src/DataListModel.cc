
#include "DataListModel.hh"

#include "DataColumn.hh"

#define MILOGGER_CATEGORY "kvhqc.DataListModel"
#include "HqcLogging.hh"

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

