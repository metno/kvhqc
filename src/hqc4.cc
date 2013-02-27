/*
HQC - Free Software for Manual Quality Control of Meteorological Observations

Copyright (C) 2013 met.no

Contact information:
Norwegian Meteorological Institute
Box 43 Blindern
0313 OSLO
NORWAY
email: kvalobs-dev@met.no

This file is part of HQC

HQC is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

HQC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with HQC; if not, write to the Free Software Foundation Inc.,
51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "HqcLogging.hh"
#include "hqcmain.h"
#include "hqc_paths.hh"
#include "KvMetaDataBuffer.hh"
#include "QtKvService.hh"

#include <kvcpp/corba/CorbaKvApp.h>

#include <QtCore/QLibraryInfo>
#include <QtCore/QLocale>
#include <QtCore/QTranslator>
#include <QtGui/QApplication>

#include <boost/foreach.hpp>

using kvservice::corba::CorbaKvApp;

static void installTranslations(QApplication& app, QTranslator& translator,
                                const QString& file, const QStringList& paths)
{
    BOOST_FOREACH(const QString& p, paths)
        if (translator.load(QLocale::system(), file, "_", p)) {
            LOG4HQC_INFO("hqc", "loaded '" << file << "' translations from " << p
                         << " for ui languages=" << QLocale::system().uiLanguages().join(","));
            app.installTranslator(&translator);
            return;
        }
    LOG4HQC_WARN("hqc", "failed to load '" << file << "' translations from [" 
                 << paths.join(",")
                 << "] for ui languages=" << QLocale::system().uiLanguages().join(","));
}

static void installTranslations(QApplication& app, QTranslator& translator,
                                const QString& file, const QString& path)
{
    installTranslations(app, translator, file, QStringList(path));
}

int main( int argc, char* argv[] )
{
    QApplication a( argc, argv, true );

    QCoreApplication::setOrganizationName("Meteorologisk Institutt");
    QCoreApplication::setOrganizationDomain("met.no");
    QCoreApplication::setApplicationName("Hqc");
    
    QStringList args = a.arguments();

    QString myconf = hqc::getPath(hqc::CONFDIR) + "/kvalobs.conf";
    QString log4cpp_properties = hqc::getPath(hqc::DATADIR) + "/log4cpp.properties";
    for (int i = 1; i < args.size(); ++i) {
        if (args.at(i) == "--config") {
            if (i+1 >= args.size()) {
                std::cerr << "invalid --config without filename" << std::endl;
                return 1;
            }
            i += 1;
            myconf = args.at(i);
        } else if (args.at(i) == "--log4cpp-properties") {
            if (i+1 >= args.size()) {
                std::cerr << "invalid --log4cpp-properties without filename" << std::endl;
                return 1;
            }
            i += 1;
            log4cpp_properties = args.at(i);
        }
    }

    Log4CppConfig log4cpp(log4cpp_properties.toStdString());

    QTranslator qtTranslator;
    installTranslations(a, qtTranslator, "qt", QLibraryInfo::location(QLibraryInfo::TranslationsPath));

    QTranslator mQuTranslator;
    installTranslations(a, mQuTranslator, "qUtilities", "/usr/share/metlibs/translations");
    
    const QString langDir = ::hqc::getPath(::hqc::DATADIR) + "/lang";
    QTranslator wTranslator, hTranslator;
    installTranslations(a, wTranslator, "watchrr2", langDir);
    installTranslations(a, hTranslator, "hqc",      langDir);
    
    miutil::conf::ConfSection *confSec = CorbaKvApp::readConf(myconf.toStdString());
    if (not confSec) {
        LOG4HQC_FATAL("hqc", "cannot open configuration file '" << myconf << "'");
        return 1;
    }
    
    CorbaKvApp kvapp(argc, argv, confSec);
  
    QtKvService qkvs;
    KvMetaDataBuffer kvmdbuf;
    StInfoSysBuffer stinfobuf(confSec);

    HqcMainWindow mw;
    QString captionSuffix = QString::fromStdString(kvapp.kvpathInCorbaNameserver());
    QString caption = "HQC " + captionSuffix;
    mw.setCaption( caption );
    mw.setIcon( QPixmap( hqc::getPath(hqc::IMAGEDIR) + "/hqc.png") );
    a.setMainWidget(&mw);
    mw.startup();
    
    a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));
    return a.exec();
}
