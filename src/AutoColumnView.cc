
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

namespace /* anonymous */ {
template<typename T>
std::vector<T>& operator<<(std::vector<T>& container, const T& t)
{ container.push_back(t); return container; }
} // anonymous namespace

AutoColumnView::Sensors_t AutoColumnView::defaultSensors()
{
    METLIBS_LOG_SCOPE();
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
    BOOST_FOREACH(int par, neighborPar) {
      Sensor sn(s);
      sn.paramId = par;
      const std::vector<Sensor> neighbors = Helpers::findNeighbors(sn, TimeRange(mSensorTime.time, mSensorTime.time), nNeighbors);
      sensors.insert(sensors.end(), neighbors.begin(), neighbors.end());
    }
#if 0
    METLIBS_LOG_DEBUG("found " << sensors.size() << " default sensors");
    BOOST_FOREACH(const Sensor& ds, sensors) {
      METLIBS_LOG_DEBUG(LOGVAL(ds));
    }
#endif
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
