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
#include "weathertable.h"

#include "BusyIndicator.h"
#include "enums.h"
#include "fdchecktableitem.h"
#include "flagitem.h"
#include "tnchecktableitem.h"
#include "weatherdialog.h"
#include "weathertableitem.h"
#include "weathertabletooltip.h"
#include "hqc_paths.hh"

#include <kvcpp/KvApp.h>
#include <kvalobs/kvDataOperations.h>
#include <kvalobs/kvModelData.h>

#include <QtGui/qapplication.h>
#include <QtGui/qmessagebox.h>
#include <QtGui/qstatusbar.h>
#include <QtCore/qfile.h>
#include <Qt3Support/Q3TextStream>

#include <boost/assign/std/vector.hpp>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>

//#define NDEBUG
#include <cassert>

using namespace kvservice;
using namespace std;
using namespace kvalobs;
using namespace boost::assign;

static const int NC = 5;

// columns with checkboxes
static const int cbCol[]  = {2,4,22,24,26};
// columns with possible distributed values
static const int dbCol[]  = {1,2,19,20,21};
// number of decimals in respective column
static const int d1Par[]  = {
    1,1,1,1,1,0,1,1,1,0,0,1,1,1,1,1,0,0,0,1,1,1,1,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};
static const int datCol[] = {
    0,1,3,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,
    20,21,23,25,27,28,29,30,31,32,33,34,35,36,
    37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53
};
static const int NL = 54;
static const QString horizonHeaders[NL] = {
    "TA", "TAN_12", "C", "TAX_12","C", "TAN", "TAX",
    "UU", "PR", "PO", "PP", "AA", "DD","FF", "FX","FX_1", "FG","FG_1","NN", "NH", "HL",
    "RR_6", "C", "RR_12","C", "RR_24", "C", "SA", "SD",
    "EM", "VV", "WW", "V1", "V2", "V3", "W1", "W2",
    "V4", "V5", "V6", "V7", "CL", "CM", "CH", "MDIR",
    "MSPE", "HW", "HWA", "PW", "PWA", "TW", "TG", "IR", "ITR"
};

namespace Weather {
const int params[NP] = {
    211,214,216,213,215,262,178,173,177,1,61,81,86,87,83,90,15,14,55,108,
    109,110,112,18,7,273,41,31,32,33,42,43,34,36,38,40,
    23,24,22,403,404,131,134,151,154,250,221,9,12
};

  WeatherTable::WeatherTable( QWidget *parent, QString name, int type )
    : Q3Table( parent )
  {
    WeatherDialog* wd = dynamic_cast<WeatherDialog*>(parent->parent());

    synFlg sf;
    vector<synDat> dataList;
    vector<synFlg> flagList;
    QString pName(name);
    timeutil::ptime protime = timeutil::from_iso_extended_string("1900-01-01 00:00:00");

    readLimits();
    connect( this, SIGNAL(valueChanged(int,int)), SLOT(markModified(int,int)));
    if ( pName == "corr" )
      connect( wd, SIGNAL(dontStore()), SLOT(restoreOld()));

    for ( int i = 0; i < NP; i++ ) {
      parm[params[i]] = horizonHeaders[datCol[i]];
    }
    for ( int i = 0; i < NP; i++ ) {
      columnIndex[datCol[i]] = i;
    }
    for(IKvObsDataList it=wd->ldList.begin(); it!=wd->ldList.end(); it++ ) {
      IDataList dit = it->dataList().begin();
      while( dit != it->dataList().end() ) {
	kvData kvDat;
	kvDat.set(dit->stationID(),
		  dit->obstime(),
		  dit->original(),
		  dit->paramID(),
		  dit->tbtime(),
		  dit->typeID(),
		  dit->sensor(),
		  dit->level(),
		  dit->corrected(),
		  dit->controlinfo(),
		  dit->useinfo(),
		  dit->cfailed());
	kvDatList.push_back(kvDat);
	dit++;
      }
    }

    int itest = 0, jtest = 0;
    for ( WeatherDialog::SynObsList::iterator it = wd->synObsList.begin();
	  it != wd->synObsList.end(); it++) {
      if ( (*it).otime > protime ) {
	timeList.push_back((*it).otime);
	for ( int i = 0; i < NP; i++ ) {
	  if ( pName == "corr" ) {
	    sd.sdat[i] = (*it).corr[i];
	    sd.styp[i] = (*it).typeId[i];
	    sd.ssen[i] = (*it).sensor[i];
	    sd.ctrlinfo[i] = (*it).controlinfo[i];

	  }
	  else if ( pName == "orig" ) {
	    sd.sdat[i] = (*it).orig[i];
	    sd.styp[i] = (*it).typeId[i];
	    sd.ssen[i] = (*it).sensor[i];
	  }
	  sf.sflg[i] = flagText((*it).controlinfo[i]);
	}
	dataList.push_back(sd);
	flagList.push_back(sf);
	protime = (*it).otime;
	jtest++;
      }
      itest++;
    }
    setNumRows(flagList.size());
    setNumCols(NL);
    displayHorizontalHeader();
    displayVerticalHeader(timeList);
    if ( pName == "corr" || pName == "orig" ) {
      displayData(pName, dataList, flagList);
    }
    else if ( pName == "flag" )
      displayFlags(flagList);
    toolTip = new WeatherTableToolTip( this );

    BusyIndicator busy;
  }

  QString WeatherTable::flagText(const string& controlInfo)
  {
    QString flTypes[] = {"fqclevel","fr","fcc","fs","fnum","fpos","fmis","ftime",
			 "fw","fstat","fcp","fclim","fd","fpre","fcombi","fhqc"};
    QString flTyp;
    QString flText;
    QString ctrInfo(QString::fromStdString(controlInfo));
    int maxFlg = 0;
    for ( int i = 0; i < 16; i++ ) {
      int flg = ctrInfo.mid(i,1).toInt(0,16);
      if ( flg > maxFlg ) {
	maxFlg = flg;
	flTyp = flTypes[i];
      }
    }
    if ( maxFlg == 0 )
      flText = "";
    else if ( maxFlg == 1 )
      flText = "OK";
    else {
      flText = flText.setNum(maxFlg);
      flText = flTyp + " = " + flText;
    }
    return flText;
  }

  void WeatherTable::displayHorizontalHeader() {
    for ( int icol = 0; icol < NL; icol++ ) {
      horizontalHeader()->setLabel(icol, horizonHeaders[icol]);
    }
  }

  void WeatherTable::displayVerticalHeader( vector<timeutil::ptime>& timeList) {
    int irow = 0;
    for ( vector<timeutil::ptime>::iterator it = timeList.begin(); it != timeList.end(); it++) {
      const QString dateTime = QString::fromStdString(timeutil::to_iso_extended_string(*it));
      verticalHeader()->setLabel(irow,dateTime);
      ++irow;
    }
  }

  void WeatherTable::displayData(QString pName, vector<synDat>& dataList, vector<synFlg>& flagList) {
    QStringList datRow;
    vector<QStringList> datRowList;
    int iRow = 0;
    vector<synFlg>::iterator fit = flagList.begin();
    for ( vector<synDat>::iterator it = dataList.begin(); it != dataList.end(); it++) {
      for ( int iCol = 0; iCol < NP; iCol++ ) {
	QString strdat;
	QString strtyp;
       	QString strflg;
       	string ctrlinfo;
       	strflg = (*fit).sflg[iCol];
	if ( (*it).sdat[iCol] < -32000 ) {
	  strdat = "";
	  strtyp = "";
	}
	else {
	  strdat = strdat.setNum((*it).sdat[iCol],'f',d1Par[iCol]);
	  strtyp = strtyp.setNum((*it).styp[iCol]);
	}
	datRow.push_back(strdat);
	ctrlinfo = (*it).ctrlinfo[iCol];
	if ( pName == "corr" ) {
	  WeatherTableItem* datItem = new WeatherTableItem(this, Q3TableItem::OnTyping,strtyp,strdat);
	  setItem(iRow,datCol[iCol],datItem);
	  if ( strflg == "fnum = 6" && ctrlinfo.substr(15,1) <= "1" )
	    datItem->isModelVal = true;
	  else
	    datItem->isModelVal = false;

	  if ( !ctrlinfo.empty() && (ctrlinfo.substr(7,1) > "0" ||
		  ctrlinfo.substr(8,1) > "1" ||
		  ctrlinfo.substr(9,1) > "1" ||
		  ctrlinfo.substr(11,1) > "1" ||
		  ctrlinfo.substr(12,1) > "6") )

	    datItem->isCorrectedByQC2 = true;
	  else
	    datItem->isCorrectedByQC2 = false;
	}
	else if ( pName == "orig" ) {
	  WeatherTableItem* datItem = new WeatherTableItem(this, Q3TableItem::Never,strtyp,strdat);
	  datItem->isModelVal = false;
	  datItem->isCorrectedByQC2 = false;
	  setItem(iRow,datCol[iCol],datItem);
	}
      }

      for ( int iCol = 0; iCol < NC; iCol++ ) {
       	QString strflg;
	//       	strflg = (*fit).sflg[dbCol[iCol]]
       	strflg = strflg.setNum((*it).styp[dbCol[iCol]]);
	if ( dbCol[iCol] < 3 ) {
	  TnCheckTableItem* ctItem = new TnCheckTableItem(this, strflg);
	  setItem(iRow,cbCol[iCol],ctItem);
	}
	else {
	  FdCheckTableItem* ctItem = new FdCheckTableItem(this, strflg);
	  setItem(iRow,cbCol[iCol],ctItem);
	}
     }

      datRowList.push_back(datRow);
      datRow.clear();
      iRow++;
      fit++;
    }
   for ( int icol = 0; icol < NL; icol++ )
      adjustColumn(icol);

    bool bdat[NL];
    for ( int icol = 0; icol < NP; icol++ ) {
      bdat[icol] = false;
      for ( int irow = 0; irow < iRow; irow++ ) {
	if ( !datRowList[irow][icol].isEmpty() ) {
	  bdat[icol] = true;
	  break;
	}
      }
      if ( !bdat[icol] )
	hideColumn(datCol[icol]);
    }

  }

  void WeatherTable::displayFlags(vector<synFlg>& flagList) {
    cerr << "Flaglist size 2 = " << flagList.size() << endl;
    QStringList flagRow;
    vector<QStringList> flagRowList;
    int iRow = 0;
    for ( vector<synFlg>::iterator it = flagList.begin(); it != flagList.end(); it++) {
      for ( int iCol = 0; iCol < NP; iCol++ ) {
	QString strdat;
	strdat = (*it).sflg[iCol];
	flagRow.push_back(strdat);
	FlagItem* flgItem = new FlagItem(this, Q3TableItem::Never,"",strdat);
	setItem(iRow,datCol[iCol],flgItem);
      }
      flagRowList.push_back(flagRow);
      flagRow.clear();
      iRow++;
    }

    for ( int icol = 0; icol < NP; icol++ )
      adjustColumn(icol);
    for ( int icol = 0; icol < NC; icol++ )
      hideColumn(cbCol[icol]);

    bool bfl[NL];
    for ( int icol = 0; icol < NP; icol++ ) {
      bfl[icol] = false;
      for ( int irow = 0; irow < iRow; irow++ ) {
	if ( !flagRowList[irow][icol].isEmpty() ) {
	  bfl[icol] = true;
	  break;
	}
      }
      if ( !bfl[icol] )
	hideColumn(datCol[icol]);
    }
  }

  void WeatherTable::getModifiedData( DataConsistencyVerifier::DataSet& /*mod*/ )
  {
  }

  void WeatherTable::markModified( int row, int col )
  {
    const double epsilon = 0.05;
    Q3TableItem *tit = item( row, col );
    float newCorr = (tit->text()).toFloat();
    kvData kvDat = getKvData(row, col);
    if ( kvDat.stationID() == 0 ) {
      QMessageBox::information( this,
				tr("Kan ikke lagre"),
				tr("Raden fins ikke i databasen"),
				QMessageBox::Ok,
				QMessageBox::NoButton );
      return;
    }

    std::list<kvalobs::kvObsPgm> obsPgmList;
    bool ok = kvservice::KvApp::kvApp->getKvObsPgm( obsPgmList, std::list<long>(), false );

    int typ = kvDat.typeID();

    if ( abs(kvDat.typeID()) > 503 || kvDat.typeID() == 0 )
      typ =findTypeId(kvDat.typeID(),kvDat.stationID(),kvDat.paramID(),timeutil::from_miTime(kvDat.obstime()),obsPgmList);
    if ( typ == -32767 ) {
      QMessageBox::information( this,
				tr("Ulovlig parameter"),
				tr("Denne parameteren fins ikke i obs_pgm\nfor denne stasjonen"),
				QMessageBox::Ok,
				Qt::NoButton );
      tit->setText("");
      return;
    }

    float oldCorr = kvDat.corrected();
    float org     = kvDat.original();
    QString oldCorrStr;
    oldCorrStr = oldCorrStr.setNum(oldCorr,'f',1);
    oldNewPair op(oldCorr, newCorr);
    rowColPair rc(row, col);
    kvDat.corrected(newCorr);
    kvControlInfo cif = kvDat.controlinfo();
    std::string cfailed = kvDat.cfailed();
    float uplim = highMap[kvDat.paramID()];
    float downlim = lowMap[kvDat.paramID()];
    if ( (newCorr > uplim || newCorr < downlim)
	 && newCorr != -32766
	 && newCorr != -32767 ) {
      QMessageBox::information( this,
				tr("Ulovlig verdi"),
				tr("Verdien er utenfor fysikalske grenser"),
				QMessageBox::Ok,
				QMessageBox::NoButton );
      if ( oldCorr == -32767.0 )
	tit->setText("");
      else
	tit->setText(oldCorrStr);
      return;
    }
    //    cif.set(kvalobs::flag::fhqc,1);
    if ( fabs(newCorr - org ) < epsilon ) {
      if ( cif.flag(kvalobs::flag::fnum) >= 1 ) {
	cif.set(kvalobs::flag::fhqc,1);
	//	cif.set(kvalobs::flag::fnum,1);
	//	cif.set(kvalobs::flag::fmis,0);
      }
      if ( cif.flag(kvalobs::flag::fmis) == 0 ) {
	cif.set(kvalobs::flag::fhqc,1);
	if ( cif.flag(kvalobs::flag::fd) == 3 )
	  cif.set(kvalobs::flag::fd,1);
      }
      else if ( cif.flag(kvalobs::flag::fmis) == 1 ) {
	cif.set(kvalobs::flag::fhqc,4);
	cif.set(kvalobs::flag::fmis,3);
	//	cif.set(kvalobs::flag::fnum,1);
      }
      else if ( cif.flag(kvalobs::flag::fmis) == 2 ) {
	cif.set(kvalobs::flag::fhqc,1);
	cif.set(kvalobs::flag::fmis,0);
	if ( cif.flag(kvalobs::flag::fd) == 3 )
	  cif.set(kvalobs::flag::fd,1);
      }
    }
    else
      cif.set(kvalobs::flag::fhqc,7);

    if ( newCorr == -32766.0 ) {
      cif.set(kvalobs::flag::fhqc,10);
      const int misfl = cif.flag(kvalobs::flag::fmis);
      if ( misfl == 0 || misfl == 1 )
	cif.set(kvalobs::flag::fmis,misfl + 2);
    }
    else if ( cif.flag(kvalobs::flag::fmis) == 1 || cif.flag(kvalobs::flag::fmis) == 3 && newCorr > -32766 ) {
      cif.set(kvalobs::flag::fmis,1);
      cif.set(kvalobs::flag::fhqc,5);
    }
    else if ( org > -32766.0 && (cif.flag(kvalobs::flag::fmis) == 2) ) {
      //      cif.set(kvalobs::flag::fmis,0);
      cif.set(kvalobs::flag::fmis,4);
      cif.set(kvalobs::flag::fhqc,7);
    }
    else if ( cif.flag(kvalobs::flag::fmis) == 4 ) {
      cif.set(kvalobs::flag::fhqc,7);
    }

    if ( oldCorr == -32767.0 ) {
      cif.set(kvalobs::flag::fhqc,5);                      //Interpol
      int misfl;
      if ( cif.flag(kvalobs::flag::fmis) == 0 )
	misfl = 1;
      else
	misfl  = cif.flag(kvalobs::flag::fmis) - 2;
      cif.set(kvalobs::flag::fmis,misfl);
    }
    kvDat.controlinfo(cif);

    if ( not cfailed.empty() )
      cfailed += ",";
    cfailed += "watchweather";
    kvDat.cfailed(cfailed);

    kvCorrList.push_back(kvDat);
    oldNew.push_back(op);
    rowCol.push_back(rc);
    corr.oTime = timeutil::from_miTime(kvDat.obstime());
    corr.parName = parm[datCol[col]];
    corr.oldVal = oldCorr;
    corr.newVal = newCorr;
  }

  kvData WeatherTable::getKvData( int row, int col ) {
    timeutil::ptime cTime = timeList[row];
    //Find paramID in col
    bool foundRow = false;
    int cParam = params[columnIndex[col]];
    vector<kvData>::iterator kvit;
    for ( kvit = kvDatList.begin(); kvit != kvDatList.end(); kvit++) {
      if ( (*kvit).paramID() == cParam && timeutil::from_miTime(kvit->obstime()) == cTime && (*kvit).typeID() == sd.styp[columnIndex[col]]) {
	foundRow = true;
	break;
      }
    }
    kvData kvCorrDat;
    if ( foundRow )
      kvCorrDat.set(kvit->stationID(),
		    kvit->obstime(),
		    kvit->original(),
		    kvit->paramID(),
		    kvit->tbtime(),
		    kvit->typeID(),
		    kvit->sensor(),
		    kvit->level(),
		    kvit->corrected(),
		    kvit->controlinfo(),
		    kvit->useinfo(),
		    kvit->cfailed());
    else
      kvCorrDat.set(kvDatList.begin()->stationID(),
              timeutil::to_miTime(cTime),
		    -32767,
		    cParam,
		    timeutil::to_miTime(cTime),
		    sd.styp[columnIndex[col]],
		    0,
		    0,
		    -32767,
		    kvDatList.begin()->controlinfo(),
		    kvDatList.begin()->useinfo(),
		    kvDatList.begin()->cfailed());

    return kvCorrDat;
  }

  void WeatherTable::updateStatusbar( int row, int col )
  {
    Q3TableItem *i = item( row, col );
    SelfExplainable *e = dynamic_cast<SelfExplainable*>( i );
    QString msg;
    if ( e ) {
      msg = e->explain();
    }
    else if ( i ) {
      msg = i->text();
    }
    else {
      msg = "";
    }
  }

  void WeatherTable::readLimits() {
    int par, dum;
    float low, high;
    QString limitsFile = ::hqc::getPath(::hqc::CONFDIR ) + "/slimits";
    QFile limits(limitsFile);
    if ( !limits.open(QIODevice::ReadOnly) ) {
      cerr << "kan ikke åpne " << limitsFile.toStdString() << endl;
      exit(1);
    }
    Q3TextStream limitStream(&limits);
    while ( limitStream.atEnd() == 0 ) {
      limitStream >> par >> dum >> low >> high;
      lowMap[par] = low;
      highMap[par] = high;
    }
  }

  void WeatherTable::restoreOld() {
    vector<oldNewPair>::iterator mit;
    vector<rowColPair>::iterator pit = rowCol.begin();
    for ( mit = oldNew.begin(); mit != oldNew.end(); mit++ ) {
      QString oldCorVal;
      QString newCorVal;
      oldCorVal = oldCorVal.setNum(mit->first,'f',1);
      newCorVal = newCorVal.setNum(mit->second,'f',1);
      int row = pit->first;
      int col = pit->second;
      Q3TableItem *tit = item( row, col );
      if ( oldCorVal == "-32767.0" )
	tit->setText("");
      else
	tit->setText(oldCorVal);
      setCurrentCell(row, col);
      pit++;
    }
    return;
  }

  void WeatherTable::showCurrentPage()
  {
       ensureCellVisible(currRow, 0);
       selectRow(currRow);
  }

  WeatherTable::~WeatherTable( )
  {}

  //  int WeatherTable::findTypeId(int typ, int pos, int par, timeutil::ptime oTime, ObsPgmList obsPgmList)
int WeatherTable::findTypeId(int typ, int pos, int par, const timeutil::ptime& oTime, const std::list<kvalobs::kvObsPgm>& obsPgmList)
  {
    int tpId;
    tpId = typ;
    //    for(CIObsPgmList obit=obsPgmList.end();obit!=obsPgmList.begin(); obit--){
    for(list<kvObsPgm>::const_iterator obit=obsPgmList.end();obit!=obsPgmList.begin(); obit--){
      if ( obit->stationID() == pos && obit->paramID() == par && timeutil::from_miTime(obit->fromtime()) < oTime && timeutil::from_miTime(obit->totime()) >= oTime) {
	tpId = obit->typeID();
	break;
      }
    }
    if ( abs(tpId) > 503 ) {
      switch (par) {
      case 106:
	for(list<kvObsPgm>::const_iterator obit=obsPgmList.end();obit!=obsPgmList.begin(); obit--){
	  if ( obit->stationID() == pos && obit->paramID() == 105 && timeutil::from_miTime(obit->fromtime()) < oTime) {
	    tpId = -obit->typeID();
	    break;
	  }
	}
	break;
      case 109:
	for(list<kvObsPgm>::const_iterator obit=obsPgmList.end();obit!=obsPgmList.begin(); obit--){
	  if ( obit->stationID() == pos && (obit->paramID() == 104 || obit->paramID() == 105 || obit->paramID() == 106) && timeutil::from_miTime(obit->fromtime()) < oTime) {
	    tpId = -obit->typeID();
	    break;
	  }
	}
	break;
      case 110:
	for(list<kvObsPgm>::const_iterator obit=obsPgmList.end();obit!=obsPgmList.begin(); obit--){
	  if ( obit->stationID() == pos && (obit->paramID() == 104 || obit->paramID() == 105 || obit->paramID() == 106 || obit->paramID() == 109) && timeutil::from_miTime(obit->fromtime()) < oTime) {
	    tpId = -obit->typeID();
	    break;
	  }
	}
	break;
      case 214:
	for(list<kvObsPgm>::const_iterator obit=obsPgmList.end();obit!=obsPgmList.begin(); obit--){
	  if ( obit->stationID() == pos && obit->paramID() == 213 && timeutil::from_miTime(obit->fromtime()) < oTime) {
	    tpId = -obit->typeID();
	    break;
	  }
	}
	break;
      case 216:
	for(list<kvObsPgm>::const_iterator obit=obsPgmList.end();obit!=obsPgmList.begin(); obit--){
	  if ( obit->stationID() == pos && obit->paramID() == 215 && timeutil::from_miTime(obit->fromtime()) < oTime) {
	    tpId = -obit->typeID();
	    break;
	  }
	}
	break;
      case 224:
	for(list<kvObsPgm>::const_iterator obit=obsPgmList.end();obit!=obsPgmList.begin(); obit--){
	  if ( obit->stationID() == pos && obit->paramID() == 223 && timeutil::from_miTime(obit->fromtime()) < oTime) {
	    tpId = -obit->typeID();
	    break;
	  }
	}
	break;
      default:
	tpId = -32767;
	break;
      }
    }
    return tpId;
  }
}

