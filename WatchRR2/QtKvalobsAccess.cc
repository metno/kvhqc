
#include "QtKvalobsAccess.hh"
#include "QtKvService.hh"

// #define NDEBUG
#include "debug.hh"
#ifndef NDEBUG
#include "Helpers.hh"
#include <boost/foreach.hpp>
#endif

QtKvalobsAccess::QtKvalobsAccess()
{
    kvservice::KvDataSubscribeInfoHelper dataSubscription;
    mKvServiceSubsription = qtKvService()->subscribeData(dataSubscription, this, SLOT(onKvData(kvservice::KvObsDataListPtr)));
}

QtKvalobsAccess::~QtKvalobsAccess()
{
    qtKvService()->unsubscribe(mKvServiceSubsription);
}

void QtKvalobsAccess::onKvData(kvservice::KvObsDataListPtr data)
{
#ifndef NDEBUG
    std::cout << "data notify:" << std::endl;
    BOOST_FOREACH(kvservice::KvObsData& od, *data) {
        BOOST_FOREACH(const kvalobs::kvData& kvd, od.dataList()) {
            const bool sub = isSubscribed(Helpers::sensorTimeFromKvData(kvd));
            if (sub)
            std::cout << '[' << kvd.stationID() << ' ' << timeutil::to_iso_extended_string(kvd.obstime())
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
