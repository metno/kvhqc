/*
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
#include <list>
//#include <kvQtApp.h>
#include <kvcpp/KvApp.h>
#include <kvalobs/kvData.h>
#include <kvalobs/kvStation.h>
#include <kvalobs/kvDbGate.h>
#include <string>
#include <kvdb/dbdrivermgr.h>
#include <qUtilities/QLetterCommands.h>

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
const int NOPARAMALL       = 210;
//const int NOPARAMAIRPRESS  = 23; //Ny gr.
//const int NOPARAMTEMP      = 31; //Ny gr
//const int NOPARAMPREC      = 40;
//const int NOPARAMVISUAL    = 36;
//const int NOPARAMWAVE      = 45;
//const int NOPARAMSYNOP     = 39;
//const int NOPARAMKLSTAT    = 33;
//const int NOPARAMPRIORITY  = 31;
//const int NOPARAMWIND      = 4;
//const int NOPARAMPLU       = 6;
const int noInfo = 7;
const int nnn = 1;

enum listType {erLi, erLo, daLi,erSa, alLi, dumLi};
enum mettType {tabHead, tabList};
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
