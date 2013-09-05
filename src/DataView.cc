
#include "DataView.hh"

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#define NDEBUG
#include "debug.hh"

DataView::DataView()
{
}

DataView::~DataView()
{
    if (mDA)
        mDA->obsDataChanged.disconnect(boost::bind(&DataView::onDataChanged, this, _1, _2));
}

void DataView::setDataAccess(EditAccessPtr eda, ModelAccessPtr mda)
{
    LOG_SCOPE("DataView");
    if (eda != mDA) {
        unsubscribeAll();
        if (mDA)
            mDA->obsDataChanged.disconnect(boost::bind(&DataView::onDataChanged, this, _1, _2));
        mDA = eda;
        if (mDA)
            mDA->obsDataChanged.connect(boost::bind(&DataView::onDataChanged, this, _1, _2));
    }
    mMA = mda;
    if (not mMA)
        LOG4SCOPE_DEBUG("no model access");
}

void DataView::setSensorsAndTimes(const Sensors_t& sensors, const TimeRange& limits)
{
    LOG_SCOPE("DataView");
    subscribeAll(sensors, limits);
}

void DataView::subscribeAll(const Sensors_t& sensors, const TimeRange& limits)
{
    LOG_SCOPE("DataView");
    // do not unsubscribe before subscribing for the new time limits
    Subscriptions_t newSubscriptions;
    if (mDA) {
        BOOST_FOREACH(const Sensor& s, sensors) {
            ObsSubscription sub(s, limits);
            mDA->addSubscription(sub);
            newSubscriptions.push_back(sub);
        }
    }
    unsubscribeAll();
    mSubscriptions = newSubscriptions;
}

void DataView::unsubscribeAll()
{
    LOG_SCOPE("DataView");
    if (mDA) {
        BOOST_FOREACH(const ObsSubscription sub, mSubscriptions)
            mDA->removeSubscription(sub);
    }
    mSubscriptions.clear();
}

void DataView::onDataChanged(ObsAccess::ObsDataChange what, ObsDataPtr obs)
{
    LOG_SCOPE("DataView");
    // TODO
}
