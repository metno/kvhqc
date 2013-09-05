
#include "AutoColumnView.hh"

#include "Helpers.hh"

#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.AutoColumnView"
#include "HqcLogging.hh"

AutoColumnView::AutoColumnView()
{
}

AutoColumnView::~AutoColumnView()
{
}

void AutoColumnView::navigateTo(const SensorTime& st)
{
    METLIBS_LOG_SCOPE();
    if (eq_SensorTime()(mSensorTime, st))
      return;

    if (mSensorTime.valid()) {
        // record changes
        BOOST_FOREACH(ViewInfo& vi, mViews) {
            vi.changes[mSensorTime.sensor] = vi.view->changes();
        }
    }
    mSensorTime = st;

    // navigate views to new SensorTime
    const Sensors_t defSens = defaultSensors();
    const TimeRange defLimits = defaultTimeLimits();
    BOOST_FOREACH(const ViewInfo& vi, mViews) {
        vi.view->setSensorsAndTimes(defSens, defLimits);
        Changes4ST_t::const_iterator it = vi.changes.find(mSensorTime.sensor);
        if (it != vi.changes.end())
            vi.view->replay(it->second);
        vi.view->navigateTo(mSensorTime);
    }
}

void AutoColumnView::attachView(ViewP v)
{
    mViews.push_back(ViewInfo(v));
    if (mSensorTime.valid())
        v->setSensorsAndTimes(defaultSensors(), defaultTimeLimits());
}

void AutoColumnView::detachView(ViewP v)
{
    for(Views_t::iterator it = mViews.begin(); it != mViews.end(); ++it) {
        if (it->view == v) {
            mViews.erase(it);
            return;
        }
    }
    METLIBS_LOG_WARN("cannot detach view");
}

AutoColumnView::Sensors_t AutoColumnView::defaultSensors()
{
  return Helpers::relatedSensors(mSensorTime);
}

TimeRange AutoColumnView::defaultTimeLimits()
{
    int hours = 24;
    if (mSensorTime.sensor.paramId == kvalobs::PARAMID_RR_24)
        hours = 7*24;

    const boost::posix_time::time_duration dt = boost::posix_time::hours(hours);
    const timeutil::ptime& t = mSensorTime.time;
    return TimeRange(t - dt, t + dt);
}
