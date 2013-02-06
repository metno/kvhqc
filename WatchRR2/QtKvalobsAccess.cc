
#include "QtKvalobsAccess.hh"
#include "QtKvService.hh"

#include <boost/foreach.hpp>

#define NDEBUG
#include "debug.hh"
#ifndef NDEBUG
#include "Helpers.hh"
#endif

QtKvalobsAccess::QtKvalobsAccess()
{
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

void QtKvalobsAccess::updateSubscribedTimes()
{
    KvalobsAccess::updateSubscribedTimes();

    std::string newKvServiceSubscriberID;
    if (not mSubscribedTimes.empty()) {
        kvservice::KvDataSubscribeInfoHelper dataSubscription;
        BOOST_FOREACH(SubscribedTimes_t::value_type& st, mSubscribedTimes)
            dataSubscription.addStationId(st.first);
        newKvServiceSubscriberID = qtKvService()
            ->subscribeData(dataSubscription, this, SLOT(onKvData(kvservice::KvObsDataListPtr)));
    }
    
    if (not mKvServiceSubscriberID.empty())
        qtKvService()->unsubscribe(mKvServiceSubscriberID);
    mKvServiceSubscriberID = newKvServiceSubscriberID;
}
