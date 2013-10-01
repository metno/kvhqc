
#include "QtKvalobsAccess.hh"

#include "QtKvService.hh"

#include <QtCore/QTimer>

#include <boost/foreach.hpp>
#include <boost/range/adaptor/map.hpp>

#define MILOGGER_CATEGORY "kvhqc.QtKvalobsAccess"
#define M_TIME
#include "util/HqcLogging.hh"

QtKvalobsAccess::QtKvalobsAccess()
  : mResubscribeTimer(new QTimer(this))
{
  mResubscribeTimer->setSingleShot(true);
  connect(mResubscribeTimer, SIGNAL(timeout()), this, SLOT(doReSubscribe()));
}

QtKvalobsAccess::~QtKvalobsAccess()
{
  if (not mKvServiceSubscriberID.empty())
    qtKvService()->unsubscribe(mKvServiceSubscriberID);
}

void QtKvalobsAccess::onKvData(kvservice::KvObsDataListPtr data)
{
  nextData(*data, true);
}

void QtKvalobsAccess::findRange(const std::vector<Sensor>& sensors, const TimeRange& limits)
{
  bool addedStation = false;
  BOOST_FOREACH(const Sensor& s, sensors) {
    const std::pair<stations_with_data_t::iterator, bool> ins = mStationsWithData.insert(s.stationId);
    addedStation |= ins.second;
  }
  if (addedStation)
    reSubscribe();

  KvalobsAccess::findRange(sensors, limits);
}

void QtKvalobsAccess::reSubscribe()
{
  METLIBS_LOG_SCOPE();
  mResubscribeTimer->start(100 /*ms*/);
}

void QtKvalobsAccess::doReSubscribe()
{
  if (not mKvServiceSubscriberID.empty()) {
    METLIBS_LOG_TIME();
    METLIBS_LOG_DEBUG("old id=" << mKvServiceSubscriberID);
    qtKvService()->unsubscribe(mKvServiceSubscriberID);
    mKvServiceSubscriberID = "";
  }
  
  if (not mStationsWithData.empty()) {
    kvservice::KvDataSubscribeInfoHelper dataSubscription;
    {
      METLIBS_LOG_TIME();
      METLIBS_LOG_DEBUG(LOGVAL(mStationsWithData.size()));
      BOOST_FOREACH(int sid, mStationsWithData) {
        dataSubscription.addStationId(sid);
      }
    }
    {
      METLIBS_LOG_TIME();
      mKvServiceSubscriberID = qtKvService()
          ->subscribeData(dataSubscription, this, SLOT(onKvData(kvservice::KvObsDataListPtr)));
      METLIBS_LOG_DEBUG("new id=" << mKvServiceSubscriberID);
    }
  }
}
