
#include "DataColumn.hh"

#include "Helpers.hh"
#include "FlagChange.hh"
#include "SensorHeader.hh"
#include "Tasks.hh"

#include <kvalobs/kvDataOperations.h>
#include <boost/bind.hpp>
#include <boost/make_shared.hpp>
#include <QtGui/QApplication>
#include <QtGui/QBrush>

#define NDEBUG
#include "w2debug.hh"

DataColumn::DataColumn(EditAccessPtr da, const Sensor& sensor, const TimeRange& t, DataItemPtr item)
    : mDA(da)
    , mSensor(sensor)
    , mTime(t)
    , mItem(item)
    , mHeaderShowStation(true)
{
    mDA->addSubscription(ObsSubscription(mSensor.stationId, mTime));
    mDA->obsDataChanged.connect(boost::bind(&DataColumn::onDataChanged, this, _1, _2));
}

DataColumn::~DataColumn()
{
    mDA->obsDataChanged.disconnect(boost::bind(&DataColumn::onDataChanged, this, _1, _2));
    mDA->removeSubscription(ObsSubscription(mSensor.stationId, mTime));
}

Qt::ItemFlags DataColumn::flags(const timeutil::ptime&) const
{
    return mItem->flags();
}

QVariant DataColumn::data(const timeutil::ptime& time, int role) const
{
    return mItem->data(getObs(time), role);
}

bool DataColumn::setData(const timeutil::ptime& time, const QVariant& value, int role)
{
    try {
        return mItem->setData(getObs(time), mDA, getSensorTime(time), value, role);
    } catch (std::runtime_error& e) {
        return false;
    }
}

QVariant DataColumn::headerData(Qt::Orientation orientation, int role) const
{
    SensorHeader sh(mSensor, mHeaderShowStation ? SensorHeader::ALWAYS : SensorHeader::TOOLTIP,
                    SensorHeader::ALWAYS, mTimeOffset.hours());
    return sh.sensorHeader(mItem, orientation, role);
}

bool DataColumn::onDataChanged(ObsAccess::ObsDataChange what, ObsDataPtr obs)
{
    LOG_SCOPE();
    const SensorTime st(obs->sensorTime());
    if (not eq_Sensor()(st.sensor, mSensor))
        return false;
    const timeutil::ptime timeo = st.time - mTimeOffset;
    ObsCache_t::iterator it = mObsCache.find(timeo);
    if (it == mObsCache.end())
        return false;
    
    mObsCache.erase(it);
    columnChanged(timeo, this);
    return true;
}

EditDataPtr DataColumn::getObs(const timeutil::ptime& time) const
{
    ObsCache_t::iterator it = mObsCache.find(time);
    EditDataPtr obs;
    if (it == mObsCache.end()) {
        obs = mDA->findE(getSensorTime(time));
        mObsCache[time] = obs;
    } else {
        obs = it->second;
    }
    return obs;
}

void DataColumn::setTimeOffset(const boost::posix_time::time_duration& timeOffset)
{
    const TimeRange oldTime = mTime;
    mTime.shift(-mTimeOffset);
    mTimeOffset = timeOffset;
    mTime.shift(mTimeOffset);

    mDA->addSubscription(ObsSubscription(mSensor.stationId, mTime));
    mDA->removeSubscription(ObsSubscription(mSensor.stationId, oldTime));
}
