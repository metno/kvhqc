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

#include "HqcAppWindow.hh"
#include "common/KvalobsReinserter.hh"
#include "common/KvalobsUpdateListener.hh"
#include "common/KvMetaDataBuffer.hh"
#include "common/KvServiceHelper.hh"
#include "common/QtKvService.hh"
#include "common/StInfoSysBuffer.hh"
#include "common/HqcApplication.hh"
#include "util/hqc_paths.hh"
#include "util/Milog4cpp.hh"

#include <kvcpp/KvApp.h>
#include <miconfparser/confparser.h>
#include <miconfparser/confsection.h>

#include <QSplashScreen>

#include <iostream>

#define MILOGGER_CATEGORY "kvhqc.main"
#include "util/HqcLogging.hh"

int main( int argc, char* argv[] )
{
  // this seems to be necessary to prevent kde image plugins / libkdecore
  // from resetting LC_NUMERIC from the environment; image plugins might,
  // e.g., be loaded when the clipboard is accessed
  setenv("LC_NUMERIC", "C", 1);
  setenv("LC_ALL", "C", 1);

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
      return 0;
    }
  }

  milogger::LoggingConfig log4cpp(log4cpp_properties);
#if 1
  { // initialize milog wrapper
    milog::LogStream* milogstream = new Milog4cppStream();
    if (milog::LogManager::createLogger(MILOGGER_CATEGORY, milogstream) ) {
      milog::LogManager::setDefaultLogger(MILOGGER_CATEGORY);
    } else {
      METLIBS_LOG_WARN("cannot create milog adapter logger");
      delete milogstream;
    }
  }
#endif

  std::shared_ptr<miutil::conf::ConfSection> confSec(miutil::conf::ConfParser::parse(myconf));
  if (!confSec) {
    HQC_LOG_ERROR("cannot open configuration file '" << myconf << "'");
    return 1;
  }

  std::shared_ptr<kvservice::KvApp> kvApp(kvservice::KvApp::create("kvhqc", argc, argv, confSec));
  KvServiceHelper kvsh(kvApp);
  QtKvService qkvs(kvApp);
  KvalobsUpdateListener kul;
  KvMetaDataBuffer kvmdbuf;
  StInfoSysBuffer stinfobuf(confSec);

  HqcApplication hqc(argc, argv, confSec);

  hqc.setReinserter(std::make_shared<KvalobsReinserter>(kvApp));

  QPixmap pixmap("icons:hqc_splash.svg");
  QSplashScreen splash(pixmap);
  splash.show();
  hqc.processEvents();

  std::unique_ptr<HqcAppWindow> aw(new HqcAppWindow());
  aw->startup(hqc.kvalobsDBName());

  splash.finish(aw.get());

  // FIXME "move desctructors" to aboutToQuit handler, see file:///usr/share/qt4/doc/html/qcoreapplication.html#exec
  const int r = hqc.exec();
  aw->finish();
  aw.reset(0);
  hqc.processEvents();

  qkvs.stop();
  return r;
}
