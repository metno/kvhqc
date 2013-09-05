/* -*- c++ -*-

HQC - Free Software for Manual Quality Control of Meteorological Observations

$Id$

Copyright (C) 2007 met.no

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
#ifndef HQCDEFS_H
#define HQCDEFS_H

#include <decodeutility/DataReinserter.h>
#include <kvcpp/KvApp.h>

#include "timeutil.hh"
#include <string>

const int NOPARAM          = 1043;
const int NOPARAMALL       = 210;

enum listType {erLi, erLo, daLi, erSa, alLi, alSa, dumLi};
enum messageType {Test,Synop,Metar,Autoobs,Kvalobs=5};

struct modDatl {
  modDatl()
  {
    std::fill(orig, orig + NOPARAM, -32767);
  }

  int stnr;
  timeutil::ptime otime;
  double orig[NOPARAM];
};

struct currentType {
  int stnr;
  int par;
  timeutil::pdate fDate;
  timeutil::pdate tDate;
  int cSensor;
  int cLevel;
  int cTypeId;
};

struct TxtDat {
  int stationId;
  timeutil::ptime obstime;
  std::string original;
  int paramId;
  timeutil::ptime tbtime;
  int typeId;
};

typedef kvalobs::DataReinserter<kvservice::KvApp> HqcReinserter;

#endif
