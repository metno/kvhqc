
#include "DataListModel.hh"

#include "ColumnFactory.hh"

#include <boost/foreach.hpp>

#define NDEBUG
#include "debug.hh"

DataListModel::DataListModel(EditAccessPtr eda, const Sensors_t& sensors, const TimeRange& limits)
    : ObsTableModel(eda, limits)
{
    LOG_SCOPE("DataListModel");
    BOOST_FOREACH(const Sensor& s, sensors) {
        addColumn(ColumnFactory::columnForSensor(eda, s, limits, ColumnFactory::ORIGINAL));
        addColumn(ColumnFactory::columnForSensor(eda, s, limits, ColumnFactory::NEW_CORRECTED));
    }

    ObsAccess::TimeSet allTimes;
    BOOST_FOREACH(const Sensor& s, sensors)
        eda->addAllTimes(allTimes, s, limits);
    mTimes = Times_t(allTimes.begin(), allTimes.end());

#ifndef NDEBUG
    LOG4SCOPE_DEBUG(DBG1(mTimes.size()));
    BOOST_FOREACH(const timeutil::ptime& t, mTimes)
        LOG4SCOPE_DEBUG(timeutil::to_iso_extended_string(t));
#endif
}

DataListModel::~DataListModel()
{
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
            DataColumnPtr dc = boost::static_pointer_cast<DataColumn>(getColumn(col));
            if (!dc)
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
