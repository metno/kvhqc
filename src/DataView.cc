
#include "DataView.hh"

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.DataView"
#include "HqcLogging.hh"

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
    METLIBS_LOG_SCOPE();
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
        METLIBS_LOG_DEBUG("no model access");
}

void DataView::setSensorsAndTimes(const Sensors_t& sensors, const TimeRange& limits)
{
    METLIBS_LOG_SCOPE();
    subscribeAll(sensors, limits);
}

void DataView::subscribeAll(const Sensors_t& sensors, const TimeRange& limits)
{
    METLIBS_LOG_SCOPE();
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
    METLIBS_LOG_SCOPE();
    if (mDA) {
        BOOST_FOREACH(const ObsSubscription sub, mSubscriptions)
            mDA->removeSubscription(sub);
    }
    mSubscriptions.clear();
}

void DataView::onDataChanged(ObsAccess::ObsDataChange what, ObsDataPtr obs)
{
    METLIBS_LOG_SCOPE();
    // TODO
}
