
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

void QtKvalobsAccess::addSubscription(const ObsSubscription& s)
{
  KvalobsAccess::addSubscription(s);

  SubscribedStations_t::iterator it = mSubscribedStations.find(s.stationId());
  if (it == mSubscribedStations.end()) {
    mSubscribedStations.insert(std::make_pair(s.stationId(), 1));
    reSubscribe();
  } else {
    it->second += 1;
  }
}

void QtKvalobsAccess::removeSubscription(const ObsSubscription& s)
{
  KvalobsAccess::removeSubscription(s);

  SubscribedStations_t::iterator it = mSubscribedStations.find(s.stationId());
  if (it == mSubscribedStations.end()) {
    HQC_LOG_ERROR("station " << s.stationId() << " has no subscribtions, cannot unsubscribe");
    return;
  }

  int& count = it->second;
  count -= 1;
  if (count == 0) {
    mSubscribedStations.erase(it);
    reSubscribe();
  }
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
  
  if (not mSubscribedStations.empty()) {
    kvservice::KvDataSubscribeInfoHelper dataSubscription;
    {
      METLIBS_LOG_TIME();
      METLIBS_LOG_DEBUG(LOGVAL(mSubscribedStations.size()));
      BOOST_FOREACH(int sid, boost::adaptors::keys(mSubscribedStations))
          dataSubscription.addStationId(sid);
    }
    {
      METLIBS_LOG_TIME();
      mKvServiceSubscriberID = qtKvService()
          ->subscribeData(dataSubscription, this, SLOT(onKvData(kvservice::KvObsDataListPtr)));
      METLIBS_LOG_DEBUG("new id=" << mKvServiceSubscriberID);
    }
  }
}
