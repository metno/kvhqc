
#include "KvalobsUpdateListener.hh"

#include "QtKvService.hh"

#include <kvalobs/kvData.h>

#include <QtCore/QTimer>

#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.KvalobsUpdateListener"
#include "util/HqcLogging.hh"

KvalobsUpdateListener::KvalobsUpdateListener()
  : mResubscribeTimer(new QTimer(this))
{
  METLIBS_LOG_SCOPE();
  mResubscribeTimer->setSingleShot(true);
  connect(mResubscribeTimer, SIGNAL(timeout()), this, SLOT(doReSubscribe()));
  setUpdateListener(this);
}

KvalobsUpdateListener::~KvalobsUpdateListener()
{
  METLIBS_LOG_SCOPE();
  if (not mKvServiceSubscriberID.empty())
    qtKvService()->unsubscribe(mKvServiceSubscriberID);
  if (not mSubscribedStations.empty())
    HQC_LOG_WARN("station list not empty");
  setUpdateListener(0);
}

void KvalobsUpdateListener::onKvData(kvservice::KvObsDataListPtr dl)
{
  METLIBS_LOG_SCOPE();
  BOOST_FOREACH(kvservice::KvObsData& od, *dl) {
    BOOST_FOREACH(const kvalobs::kvData& kvd, od.dataList()) {
      METLIBS_LOG_DEBUG("updated: " << kvd);
      Q_EMIT update(kvd);
    }

    const kvData_v data(od.dataList().begin(), od.dataList().end());
    Q_EMIT updated(data);
  }
}

void KvalobsUpdateListener::addStation(int stationId)
{
  METLIBS_LOG_SCOPE(LOGVAL(stationId));
  station_count_t::iterator it = mSubscribedStations.find(stationId);
  if (it == mSubscribedStations.end()) {
    mSubscribedStations.insert(std::make_pair(stationId, 1));
    reSubscribe();
  } else {
    it->second += 1;
  }
}

void KvalobsUpdateListener::removeStation(int stationId)
{
  METLIBS_LOG_SCOPE(LOGVAL(stationId));
  station_count_t::iterator it = mSubscribedStations.find(stationId);
  if (it == mSubscribedStations.end()) {
    HQC_LOG_WARN("station not listed");
    return;
  }
  if (it->second > 1) {
    it->second -= 1;
  } else {
    mSubscribedStations.erase(it);
    reSubscribe();
  }
}

void KvalobsUpdateListener::reSubscribe()
{
  METLIBS_LOG_SCOPE();
  mResubscribeTimer->start(100 /*ms*/);
}

void KvalobsUpdateListener::doReSubscribe()
{
  METLIBS_LOG_SCOPE();
  if (not mKvServiceSubscriberID.empty()) {
    qtKvService()->unsubscribe(mKvServiceSubscriberID);
    mKvServiceSubscriberID = "";
  }
  
  if (not mSubscribedStations.empty()) {
    kvservice::KvDataSubscribeInfoHelper dataSubscription;
    BOOST_FOREACH(const station_count_t::value_type& sc, mSubscribedStations)
        dataSubscription.addStationId(sc.first);

    mKvServiceSubscriberID = qtKvService()
        ->subscribeData(dataSubscription, this, SLOT(onKvData(kvservice::KvObsDataListPtr)));
    METLIBS_LOG_DEBUG("new id=" << mKvServiceSubscriberID);
  }
}
