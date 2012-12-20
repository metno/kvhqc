
#include "Helpers.hh"
#include "MainDialog.hh"

#define LOAD_DECL_ONLY
#include "load_31850_20121130.cc"
#include "load_32780_20121207.cc"
#include "load_44160_20121207.cc"
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

    const Sensor sensor(stationId, kvalobs::PARAMID_RR, 0, 0, 302);
    const boost::gregorian::date_duration t0_offset = boost::gregorian::days(3);
    TimeRange time;

    switch(stationId) {
    case 31850: time = t_31850_20121130(); break;
    case 32780: time = t_32780_20121207(); break;
    case 44160: time = t_44160_20121207(); break;
    case 54420: time = t_54420_20121130(); break;
    case 84070: time = t_84070_20120930(); break;
    default:
        return 1;
    }
    if (stationId != 32780)
        time = TimeRange(time.t0()+t0_offset, time.t1());

    FakeDataAccessPtr fda = boost::make_shared<FakeDataAccess>();
    FakeModelAccessPtr fma = boost::make_shared<FakeModelAccess>();
    load_31850_20121130(fda, fma);
    load_32780_20121207(fda);
    load_44160_20121207(fda);
    load_54420_20121130(fda);
    load_84070_20120930(fda, fma);
    EditAccessPtr eda = boost::make_shared<EditAccess>(fda);

    MainDialog main(eda, fma, sensor, time);
    if (main.exec())
        eda->sendChangesToParent();

    return 0;
}
