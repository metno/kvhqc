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

#include "KvalobsData.h"

#include <kvalobs/kvData.h>
#include <kvalobs/kvModelData.h>
#include <kvalobs/kvObsPgm.h>
#include <kvalobs/kvStation.h>

#include <qUtilities/QLetterCommands.h>

#include <list>
#include <string>

typedef std::list<kvalobs::kvData>                              DataList;
typedef std::list<kvalobs::kvData>::iterator                   IDataList;
typedef std::list<kvalobs::kvData>::const_iterator            CIDataList;
typedef std::list<kvalobs::kvModelData>                    ModelDataList;
typedef std::list<kvalobs::kvModelData>::iterator         IModelDataList;
typedef std::list<kvalobs::kvModelData>::const_iterator  CIModelDataList;

typedef std::list<kvalobs::kvObsPgm>                          ObsPgmList;
typedef std::list<kvalobs::kvObsPgm>::const_iterator        CIObsPgmList;

typedef std::list<int>                                 TypeList;
typedef std::list<TypeList>                         ObsTypeList;

const int NOPARAM          = 1043;
const int NOPARAMMODEL     = 8;
extern const int modelParam[NOPARAMMODEL]; // defined in hqcmain.cc
const int NOPARAMALL       = 210;

enum listType {erLi, erLo, daLi,erSa, alLi, dumLi};
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
  //  QString status;
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

#endif
