
#include "QtKvalobsAccess.hh"
#include "QtKvService.hh"

#include <QtCore/QTimer>

#include <boost/foreach.hpp>
#include <boost/range/adaptor/map.hpp>

#define MILOGGER_CATEGORY "kvhqc.QtKvalobsAccess"
#define M_TIME
#include "HqcLogging.hh"
#ifndef NDEBUG
#include "Helpers.hh"
#endif

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
#ifndef NDEBUG
    std::cout << "data notify:" << std::endl;
    BOOST_FOREACH(kvservice::KvObsData& od, *data) {
        BOOST_FOREACH(const kvalobs::kvData& kvd, od.dataList()) {
            const bool sub = isSubscribed(Helpers::sensorTimeFromKvData(kvd));
            if (sub)
            std::cout << '[' << kvd.stationID() << ' ' << timeutil::to_iso_extended_string(timeutil::from_miTime(kvd.obstime()))
                      << " p:" << std::setw(4) << kvd.paramID()
                      << " l:" << kvd.level()
                      << " s:" << (kvd.sensor() % '0')
                      << " t:" << std::setw(3) << kvd.typeID()
                      << " o:" << std::setw(8) << kvd.original()
                      << " c:" << std::setw(8) << kvd.corrected()
                      << " ci:" << kvd.controlinfo().flagstring()
                      << " ui:" << kvd.useinfo().flagstring()
                      << " cf:'" << kvd.cfailed() << "'"
                      << " sub=" << (sub ? "y" : "n") << "]"
                      << std::endl;
        }
    }
#endif

    nextData(*data);
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
    METLIBS_LOG_ERROR("station " << s.stationId() << " has no subscribtions, cannot unsubscribe");
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
  mResubscribeTimer->start(500 /*ms*/);
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
    {METLIBS_LOG_TIME();
    mKvServiceSubscriberID = qtKvService()
        ->subscribeData(dataSubscription, this, SLOT(onKvData(kvservice::KvObsDataListPtr)));
    METLIBS_LOG_DEBUG("new id=" << mKvServiceSubscriberID);
    }
  }
}
