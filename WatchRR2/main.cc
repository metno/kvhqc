
#include "Helpers.hh"
#include "HqcLogging.hh"
#include "hqc_paths.hh"
#include "identifyUser.h"
#include "KvalobsModelAccess.hh"
#include "KvMetaDataBuffer.hh"
#include "MainDialog.hh"
#include "QtKvalobsAccess.hh"
#include "QtKvService.hh"
#include "StationDialog.hh"

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

    Log4CppConfig log4cpp("-.!!=-:");

    QTranslator qtTranslator;
    qtTranslator.load(QLocale::system(), "qt", "_",
                      QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(&qtTranslator);

    const QString langDir = ::hqc::getPath(::hqc::DATADIR) + "/lang";
    QTranslator wTranslator;
    const bool translationsLoaded = wTranslator.load(QLocale::system(), "watchrr2", "_", langDir);
    if (not translationsLoaded)
        qDebug() << "failed to load translations from " << langDir;
    a.installTranslator(&wTranslator);
    
    QStringList args = a.arguments();
    
    QString myconf = "kvalobs.conf";
    Sensor sensor(83880, kvalobs::PARAMID_RR_24, 0, 0, 302);

    const timeutil::ptime now = timeutil::now();
    timeutil::ptime timeTo = timeutil::ptime(now.date(), boost::posix_time::hours(6)), timeFrom = timeTo - boost::gregorian::days(21);

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
            timeFrom = timeutil::from_iso_extended_string(args.at(i).toStdString());
            i += 1;
            timeTo = timeutil::from_iso_extended_string(args.at(i).toStdString());
        } else if (args.at(i) == "--type") {
            if (i+1 >= args.size()) {
                qDebug() << "invalid --type without typeid";
                exit(1);
            }
            i += 1;
            sensor.typeId = args.at(i).toInt();
        }
    }
    
    miutil::conf::ConfSection *confSec = kvservice::corba::CorbaKvApp::readConf(myconf.toStdString());
    if(!confSec) {
        qDebug() << "Can't open configuration file: " << myconf << endl;
        return 1;
    }

    kvservice::corba::CorbaKvApp kvapp(argc, argv, confSec);
    QtKvService qkvs;
    KvMetaDataBuffer kvsb;

    boost::shared_ptr<KvalobsAccess> kda = boost::make_shared<QtKvalobsAccess>();
    boost::shared_ptr<KvalobsModelAccess> kma = boost::make_shared<KvalobsModelAccess>();

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
            return 1;
    }
    kda->setReinserter(reinserter);
    TimeRange time(timeFrom, timeTo);

    while (true) {
        StationDialog sd(sensor, time);
        if (not sd.exec())
            break;

        sensor = sd.selectedSensor();
        time = sd.selectedTime();
        
        EditAccessPtr eda = boost::make_shared<EditAccess>(kda);
        MainDialog main(eda, kma, sensor, time);
        if (main.exec()) {
            if (not eda->sendChangesToParent()) {
                QMessageBox::critical(0,
                                      qApp->translate("Main", "WatchRR"),
                                      qApp->translate("Main", "Sorry, your changes could not be saved and are lost!"),
                                      qApp->translate("Auth", "Exit"),
                                      "");
            } else {
                QMessageBox::information(0,
                                     qApp->translate("Main", "WatchRR"),
                                         qApp->translate("Main", "Your changes have been saved."),
                                         qApp->translate("Auth", "Exit"),
                                         "");
            }
        }
    }

    return 0;
}
