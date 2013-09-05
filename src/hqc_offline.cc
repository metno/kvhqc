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
#include "KvMetaDataBuffer.hh"
#include "QtKvService.hh"
#include "StInfoSysBuffer.hh"

#include "test/FakeKvApp.hh"
#include "test/FakeReinserter.hh"

#define LOAD_DECL_ONLY
#include "test/load_examples_201303.cc"
#include "test/load_1650_20130130.cc"
#include "test/load_31850_20121130.cc"
#include "test/load_32780_20121207.cc"
#include "test/load_44160_20121207.cc"
#include "test/load_52640_20121231.cc"
#include "test/load_54420_20121130.cc"
#include "test/load_84070_20120930.cc"

int main( int argc, char* argv[] )
{
    Log4CppConfig log4cpp("!§%$!§%§");

    // >>>>> move to HqcApplication somehow
    FakeKvApp fa;
    load_examples_201303(fa);
    load_1650_20130130(fa);
    load_31850_20121130(fa);
    load_32780_20121207(fa);
    load_44160_20121207(fa);
    load_52640_20121231(fa);
    load_54420_20121130(fa);
    load_84070_20120930(fa);
    QtKvService qkvs;
    KvMetaDataBuffer kvmdbuf;
    StationInfoBuffer stinfobuf;
    // <<<<<

    HqcApplication hqc(argc, argv);
    
    hqc.setReinserter(new FakeReinserter, "fake");
    hqc.startup("offline test");

    return hqc.exec();
}
