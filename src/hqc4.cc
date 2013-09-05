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

#include "HqcApplication.hh"
#include "hqcmain.h"
#include "hqc_paths.hh"
#include "identifyUser.h"
#include "KvMetaDataBuffer.hh"
#include "QtKvService.hh"
#include "StInfoSysBuffer.hh"

#include <kvcpp/corba/CorbaKvApp.h>

#include <iostream>

#define MILOGGER_CATEGORY "kvhqc.main"
#include "HqcLogging.hh"

using kvservice::corba::CorbaKvApp;

int main( int argc, char* argv[] )
{
    std::string myconf = (hqc::getPath(hqc::CONFDIR) + "/kvalobs.conf").toStdString();
    std::string log4cpp_properties = (hqc::getPath(hqc::DATADIR) + "/log4cpp.properties").toStdString();
    for (int i = 1; i<argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--config") {
            if (i+1 >= argc) {
                std::cerr << "invalid --config without filename" << std::endl;
                return 1;
            }
            i += 1;
            myconf = argv[i];
        } else if (arg == "--log4cpp-properties") {
            if (i+1 >= argc) {
                std::cerr << "invalid --log4cpp-properties without filename" << std::endl;
                return 1;
            }
            i += 1;
            log4cpp_properties = argv[i];
        } else if (arg == "--version") {
          std::cout << PVERSION_FULL << std::endl;
          ::exit(0);
        }
    }

    milogger::LoggingConfig log4cpp(log4cpp_properties);

    miutil::conf::ConfSection *confSec = CorbaKvApp::readConf(myconf);
    if (not confSec) {
        METLIBS_LOG_ERROR("cannot open configuration file '" << myconf << "'");
        return 1;
    }
    
    CorbaKvApp kvapp(argc, argv, confSec);
    QtKvService qkvs;
    KvMetaDataBuffer kvmdbuf;
    StInfoSysBuffer stinfobuf(confSec);

    HqcApplication hqc(argc, argv, confSec);

    QString userName = "?";
    HqcReinserter* r = Authentication::identifyUser(0, kvservice::KvApp::kvApp, "ldap-oslo.met.no", userName);

    std::auto_ptr<HqcMainWindow> mw(new HqcMainWindow());
    mw->setReinserter(r, userName);
    mw->startup(QString::fromStdString(kvapp.kvpathInCorbaNameserver()));

    // FIXME "move desctructors" to aboutToQuit handler, see file:///usr/share/qt4/doc/html/qcoreapplication.html#exec
    return hqc.exec();
}
