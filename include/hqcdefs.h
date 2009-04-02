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
#include <list>
//#include <kvQtApp.h>
#include <kvcpp/KvApp.h>
#include <kvalobs/kvData.h>
#include <kvalobs/kvStation.h>
#include <kvalobs/kvDbGate.h>
#include <puTools/miTime>
#include <string>
#include <kvdb/dbdrivermgr.h>
#include <qUtilities/QLetterCommands.h>

using namespace std;
using namespace kvalobs;
using namespace kvservice;

typedef list<kvData>                              DataList;
typedef list<kvData>::iterator                   IDataList;
typedef list<kvData>::const_iterator            CIDataList;
typedef list<kvModelData>                    ModelDataList;
typedef list<kvModelData>::iterator         IModelDataList;
typedef list<kvModelData>::const_iterator  CIModelDataList;

typedef list<kvObsPgm>                          ObsPgmList;
typedef list<kvObsPgm>::const_iterator        CIObsPgmList;

typedef list<int>                                 TypeList;
typedef list<TypeList>                         ObsTypeList;

const int NOPARAM          = 1043;
const int NOPARAMMODEL     = 8;
const int NOPARAMALL       = 210;
const int NOPARAMAIRPRESS  = 23; //Ny gr.
const int NOPARAMTEMP      = 31; //Ny gr
//const int NOPARAMOTHERTEMP = 12;
//const int NOPARAMHUM       = 21;
//const int NOPARAMWIND      = 17;
//const int NOPARAMPREC      = 19;
const int NOPARAMPREC      = 40;
const int NOPARAMVISUAL    = 36;
//const int NOPARAMSIGHT     = 25;
//const int NOPARAMWEATHER   = 33;
//const int NOPARAMWAVE      = 18;
const int NOPARAMWAVE      = 45;
const int NOPARAMSYNOP     = 39;
//const int NOPARAMKLSTAT    = 17;
//const int NOPARAMKLSTAT    = 30;
const int NOPARAMKLSTAT    = 33;
//const int NOPARAMPRIORITY  = 25;
const int NOPARAMPRIORITY  = 31;
const int NOPARAMWIND      = 4;
const int NOPARAMPLU       = 6;
const int noInfo = 7;
const int nnn = 1;

enum listType {erLi, erLo, daLi,erSa, alLi, dumLi};
enum mettType {tabHead, tabList};
enum messageType {Test,Synop,Metar,Autoobs,Kvalobs=5};

struct datl {
  int stnr;
  int snr;
  QString name;
  miutil::miTime otime;
  miutil::miTime tbtime;
  //  int typeId;
  int showTypeId;
  int typeIdChanged;
  int typeId[NOPARAM];
  double orig[NOPARAM];
  int flag[NOPARAM];
  double corr[NOPARAM];
  int level[NOPARAM];
  int sensor[NOPARAM];
  string controlinfo[NOPARAM];
  string useinfo[NOPARAM];
  string cfailed[NOPARAM];
};
struct modDatl {
  int stnr;
  miutil::miTime otime;
  double orig[NOPARAM];
};
struct currentType {
  int stnr;
  //  QString status;
  int par;
  miutil::miDate fDate;
  miutil::miDate tDate;
  int cSensor;
  int cLevel;
  int cTypeId;
};
#endif
