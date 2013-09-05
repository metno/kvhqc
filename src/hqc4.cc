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
#include "HqcLogging.hh"
#include "hqc_paths.hh"
#include "identifyUser.h"
#include "KvMetaDataBuffer.hh"
#include "QtKvService.hh"

#include <kvcpp/corba/CorbaKvApp.h>

using kvservice::corba::CorbaKvApp;

int main( int argc, char* argv[] )
{
    HqcApplication hqc(argc, argv);
    const QStringList args = hqc.arguments();

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

    miutil::conf::ConfSection *confSec = CorbaKvApp::readConf(myconf.toStdString());
    if (not confSec) {
        LOG4HQC_FATAL("hqc", "cannot open configuration file '" << myconf << "'");
        return 1;
    }
    
    CorbaKvApp kvapp(argc, argv, confSec);
    QtKvService qkvs;
    KvMetaDataBuffer kvmdbuf;
    StInfoSysBuffer stinfobuf(confSec);

    QString userName = "?";
    HqcReinserter* r = Authentication::identifyUser(0, kvservice::KvApp::kvApp, "ldap-oslo.met.no", userName);
    hqc.setReinserter(r, userName);

    hqc.startup(QString::fromStdString(kvapp.kvpathInCorbaNameserver()));

    return hqc.exec();
}
