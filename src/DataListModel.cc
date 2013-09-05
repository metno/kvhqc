
#include "DataListModel.hh"

#include "ColumnFactory.hh"

#include <boost/foreach.hpp>

DataListModel::DataListModel(EditAccessPtr eda, const Sensors_t& sensors, const TimeRange& limits)
    : ObsTableModel(eda, limits)
{
    ObsAccess::TimeSet allTimes;
    BOOST_FOREACH(const Sensor& s, sensors)
        eda->addAllTimes(allTimes, s, limits);
    mTimes = Times_t(allTimes.begin(), allTimes.end());

    BOOST_FOREACH(const Sensor& s, sensors) {
        addColumn(ColumnFactory::columnForSensor(eda, s, limits, ColumnFactory::ORIGINAL));
        addColumn(ColumnFactory::columnForSensor(eda, s, limits, ColumnFactory::NEW_CORRECTED));
    }
}

DataListModel::~DataListModel()
{
}

timeutil::ptime DataListModel::timeAtRow(int row) const
{
    return mTimes.at(row);
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
