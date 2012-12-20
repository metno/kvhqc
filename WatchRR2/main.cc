
#include "Helpers.hh"
#include "identifyUser.h"
#include "MainDialog.hh"
#include "StationDialog.hh"
#include "KvalobsAccess.hh"
#include "KvalobsModelAccess.hh"

#include <kvcpp/corba/CorbaKvApp.h>

#include <QtCore/qdebug.h>
#include <QtCore/QLibraryInfo>
#include <QtCore/QStringList>
#include <QtCore/QTranslator>
#include <QtGui/QApplication>
#include <QtGui/QMessageBox>

#include <boost/make_shared.hpp>

int main(int argc, char* argv[])
{
    QApplication a( argc, argv, true );

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(),
                      QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(&qtTranslator);

    QTranslator wTranslator;
#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 0))
    if (not wTranslator.load(QLocale::system(), "watchrr2", "_", "build/WatchRR2"))
        qDebug() << "failed to load translations";
#else
    if (not wTranslator.load("watchrr2_" + QLocale::system().name(), "build/WatchRR2"))
        qDebug() << "failed to load translations";
#endif
    a.installTranslator(&wTranslator);
    
    QStringList args = a.arguments();
    
    bool haveUnknownOptions = false;
    QString myconf = "kvalobs.conf";

    Sensor sensor(54420, kvalobs::PARAMID_RR, 0, 0, 302);
    std::string timeFrom("2012-10-01T06:00:00"), timeTo("2012-11-20T06:00:00");

    for (int i = 1; i < args.size(); ++i) {
        if( args.at(i) == "--config" ) {
            if( i+1 >= args.size() ) {
                qDebug() << "invalid --config without filename";
                exit(1);
            }
            i += 1;
            myconf = args.at(i);
        } else if( args.at(i) == "--station" ) {
            if( i+1 >= args.size() ) {
                qDebug() << "invalid --station without station number";
                exit(1);
            }
            i += 1;
            sensor.stationId = args.at(i).toInt();
        } else if( args.at(i) == "--time" ) {
            if( i+2 >= args.size() ) {
                qDebug() << "invalid --time without time-from time-to";
                exit(1);
            }
            i += 1;
            timeFrom = args.at(i).toStdString();
            i += 1;
            timeTo = args.at(i).toStdString();
        } else if( args.at(i) == "--type" ) {
            if( i+1 >= args.size() ) {
                qDebug() << "invalid --type without typeid";
                exit(1);
            }
            i += 1;
            sensor.typeId = args.at(i).toInt();
        } else {
            haveUnknownOptions = true;
            qDebug() << "Unknown option: " << args.at(i);
        }
    }
    if( haveUnknownOptions ) {
        qDebug() << "have unknown options, stop";
        exit(1);
    }
    
    miutil::conf::ConfSection *confSec = kvservice::corba::CorbaKvApp::readConf(myconf.toStdString());
    if(!confSec) {
        qDebug() << "Can't open configuration file: " << myconf << endl;
        return 1;
    }

    kvservice::corba::CorbaKvApp kvapp(argc, argv, confSec);

    boost::shared_ptr<KvalobsAccess> kda = boost::make_shared<KvalobsAccess>();
    boost::shared_ptr<KvalobsModelAccess> kma = boost::make_shared<KvalobsModelAccess>();
    EditAccessPtr eda = boost::make_shared<EditAccess>(kda);

    QString userName;
    kvalobs::DataReinserter<kvservice::KvApp>* reinserter
        = Authentication::identifyUser(0, kvservice::KvApp::kvApp, "ldap-oslo.met.no", userName);
    if (not reinserter) {
        int mb = QMessageBox::information(0,
                                          qApp->translate("Auth", "Authentication"),
                                          qApp->translate("Auth", "You are not registered as operator, so you cannot save data!"),
                                          qApp->translate("Auth", "Continue"),
                                          qApp->translate("Auth", "Exit"),
                                          "");
        if (mb)
            exit(1);
    }
    kda->setReinserter(reinserter);

    TimeRange time(timeutil::from_iso_extended_string(timeFrom),
                   timeutil::from_iso_extended_string(timeTo));

    {
        StationDialog sd(sensor, time);
        if (sd.exec()) {
            sensor = sd.selectedSensor();
            time = sd.selectedTime();
        } else {
            return 1;
        }
        std::cout << "sensor = " << sensor.stationId << "/" << sensor.typeId << " time=" << time << std::endl;
    }

    MainDialog main(eda, kma, sensor, time);
    if (main.exec()) {
        if (not eda->sendChangesToParent()) {
            QMessageBox::critical(0,
                                  qApp->translate("Main", "WatchRR"),
                                  qApp->translate("Main", "Your changes could not be saved, sorry!"),
                                  qApp->translate("Auth", "Exit"),
                                  "");
        }
    }

    return 0;
}
