
#include "AutoColumnView.hh"

#include "Helpers.hh"

#include <boost/foreach.hpp>

#define NDEBUG
#include "debug.hh"

AutoColumnView::AutoColumnView()
{
}

AutoColumnView::~AutoColumnView()
{
}

void AutoColumnView::navigateTo(const SensorTime& st)
{
    if (mSensorTime.valid()) {
        // record changes
        BOOST_FOREACH(ViewInfo& vi, mViews) {
            vi.changes[mSensorTime.sensor] = vi.view->changes();
        }
    }

    mSensorTime = st;

    // navigate views to new SensorTime
    BOOST_FOREACH(const ViewInfo& vi, mViews) {
        vi.view->setSensorsAndTimes(defaultSensors(), defaultTimeLimits());
        Changes4ST_t::const_iterator it = vi.changes.find(mSensorTime.sensor);
        if (it != vi.changes.end())
            vi.view->replay(it->second);
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
    LOG4HQC_WARN("AutoColumnView", "cannot detach view");
}

namespace /* anonymous */ {
template<typename T, typename C>
C& operator<<(C& container, const T& t)
{ container.push_back(t); return container; }
} // anonymous namespace

AutoColumnView::Sensors_t AutoColumnView::defaultSensors()
{
    const Sensor& s = mSensorTime.sensor;

    std::vector<int> stationPar, neighborPar;
    int nNeighbors = 8;
    if (s.paramId == kvalobs::PARAMID_RR_24) {
        stationPar << 110 << 34 << 36 << 38 << 18 << 112;
        neighborPar << 110;
        nNeighbors = 4;
    } else if (s.paramId == 211 or s.paramId == 213 or s.paramId == 215) {
        stationPar << 211 << 213 << 215;
        neighborPar << s.paramId;
    } else {
        stationPar << s.paramId;
        neighborPar << s.paramId;
    }
    Sensors_t sensors;
    BOOST_FOREACH(int par, stationPar) {
        Sensor st(s);
        st.paramId = par;
        sensors << st;
    }
    const std::vector<Sensor> neighbors = Helpers::findNeighbors(s, TimeRange(mSensorTime.time, mSensorTime.time), nNeighbors);
    BOOST_FOREACH(const Sensor& n, neighbors) {
        BOOST_FOREACH(int par, neighborPar) {
            Sensor sn(n);
            sn.paramId = par;
            sensors << sn;
        }
    }
    return sensors;
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
