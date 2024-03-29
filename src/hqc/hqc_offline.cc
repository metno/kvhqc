/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2013-2018 met.no

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

#include "HqcMainWindow.hh"
#include "common/KvMetaDataBuffer.hh"
#include "common/KvServiceHelper.hh"
#include "common/StInfoSysBuffer.hh"
#include "common/HqcApplication.hh"
#include "util/hqc_paths.hh"

#include "common/test/FakeKvApp.hh"
#include "common/test/FakeReinserter.hh"

#define LOAD_DECL_ONLY
#include "common/test/load_examples_201303.cc"
#include "common/test/load_1650_20130130.cc"
#include "common/test/load_18210_20130410.cc"
#include "common/test/load_31850_20121130.cc"
#include "common/test/load_32780_20121207.cc"
#include "common/test/load_44160_20121207.cc"
#include "common/test/load_52640_20121231.cc"
#include "common/test/load_54420_20121130.cc"
#include "common/test/load_84070_20120930.cc"

#define MILOGGER_CATEGORY "kvhqc.offline"
#include "util/HqcLogging.hh"

int main( int argc, char* argv[] )
{
  std::string log4cpp_properties = "!§%$!§%§";
  for (int i = 1; i<argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--log4cpp-properties") {
      if (i+1 >= argc) {
        std::cerr << "invalid --log4cpp-properties without filename" << std::endl;
        return 1;
      }
      i += 1;
      log4cpp_properties = argv[i];
    }
  }
  milogger::LoggingConfig log4cpp(log4cpp_properties);

  // >>>>> move to HqcApplication somehow
  FakeKvApp fa;
//    load_examples_201303(fa);
  load_1650_20130130(fa);
  load_18210_20130410(fa);
  load_31850_20121130(fa);
  load_32780_20121207(fa);
  load_44160_20121207(fa);
  load_52640_20121231(fa);
  load_54420_20121130(fa);
  load_84070_20120930(fa);
  KvServiceHelper kvsh;
  KvMetaDataBuffer kvmdbuf;
  StationInfoBuffer stinfobuf;
  // <<<<<

  HqcApplication hqc(argc, argv, 0);

  std::unique_ptr<HqcMainWindow> mw(new HqcMainWindow());
  FakeReinserter* fri = new FakeReinserter;
  fri->setInsertSuccess(false);
  mw->setReinserter(fri);
  mw->startup("offline test");

  return hqc.exec();
}
