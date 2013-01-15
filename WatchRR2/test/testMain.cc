
#include "EditAccess.hh"
#include "FakeKvApp.hh"
#include "Helpers.hh"
#include "KvalobsModelAccess.hh"
#include "KvStationBuffer.hh"
#include "MainDialog.hh"
#include "QtKvalobsAccess.hh"
#include "QtKvService.hh"

#define LOAD_DECL_ONLY
#include "load_31850_20121130.cc"
#include "load_32780_20121207.cc"
#include "load_44160_20121207.cc"
#include "load_52640_20121231.cc"
#include "load_54420_20121130.cc"
#include "load_84070_20120930.cc"

#include <QtCore/qdebug.h>
#include <QtGui/QApplication>

#include <boost/make_shared.hpp>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv, true);

    int stationId = 54420;
    QStringList args = a.arguments();
    if (args.size() == 2)
        stationId = args[1].toInt();

    FakeKvApp fakeApp;
    QtKvService qkvs;
    KvStationBuffer kvsb;

    load_31850_20121130(fakeApp);
    load_32780_20121207(fakeApp);
    load_44160_20121207(fakeApp);
    load_52640_20121231(fakeApp);
    load_54420_20121130(fakeApp);
    load_84070_20120930(fakeApp);

    TimeRange time;
    switch(stationId) {
    case 31850: time = t_31850_20121130(); break;
    case 32780: time = t_32780_20121207(); break;
    case 44160: time = t_44160_20121207(); break;
    case 52640: time = t_52640_20121231(); break;
    case 54420: time = t_54420_20121130(); break;
    case 84070: time = t_84070_20120930(); break;
    default:
        std::cerr << "unknown stationid" << std::endl;
        return 1;
    }
    const Sensor sensor(stationId, kvalobs::PARAMID_RR, 0, 0, stationId != 52640 ? 302 : 402);
    const boost::gregorian::date_duration t0_offset = boost::gregorian::days(3);
    if (stationId != 32780)
        time = TimeRange(time.t0()+t0_offset, time.t1());

    boost::shared_ptr<KvalobsAccess> kda = boost::make_shared<QtKvalobsAccess>();
    boost::shared_ptr<KvalobsModelAccess> kma = boost::make_shared<KvalobsModelAccess>();
    EditAccessPtr eda = boost::make_shared<EditAccess>(kda);

    MainDialog main(eda, kma, sensor, time);
    if (main.exec())
        eda->sendChangesToParent();

    return 0;
}
