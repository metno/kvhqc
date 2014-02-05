
#include "QtKvalobsAccess.hh"

#include "AbstractUpdateListener.hh"

#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.QtKvalobsAccess"
#include "util/HqcLogging.hh"

QtKvalobsAccess::QtKvalobsAccess()
{
  AbstractUpdateListener* ul = updateListener();
  if (ul)
    connect(ul, SIGNAL(update(const kvalobs::kvData&)), this, SLOT(onUpdate(const kvalobs::kvData&)));
  else
    HQC_LOG_WARN("no UpdateListener");
}

QtKvalobsAccess::~QtKvalobsAccess()
{
  AbstractUpdateListener* ul = updateListener();
  if (ul) {
    BOOST_FOREACH(int sid, mStationsWithData) {
      ul->removeStation(sid);
    }
  }
}

void QtKvalobsAccess::onUpdate(const kvalobs::kvData& kvdata)
{
  receive(kvdata, true);
}

void QtKvalobsAccess::findRange(const std::vector<Sensor>& sensors, const TimeRange& limits)
{
  AbstractUpdateListener* ul = updateListener();
  BOOST_FOREACH(const Sensor& s, sensors) {
    const std::pair<stations_with_data_t::iterator, bool> ins = mStationsWithData.insert(s.stationId);
    if (ul and ins.second)
      ul->addStation(s.stationId);
  }

  KvalobsAccess::findRange(sensors, limits);
}
