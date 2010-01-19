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
#include "weatherdialog.h"
#include "enums.h"
#include "weathertabletooltip.h"
#include "BusyIndicator.h"
#include <algorithm>
#include <cmath>
#include <kvcpp/KvApp.h>
#include <kvalobs/kvDataOperations.h>
#include <kvalobs/kvModelData.h>
#include <sstream>
#include <qapplication.h>
#include <qmessagebox.h>
#include <qstatusbar.h>
#include <qfile.h>
#include <Q3TextStream>
#include <boost/assign/std/vector.hpp>

//#define NDEBUG
#include <cassert>

#include <iostream>

using namespace kvservice;
using namespace std;
using namespace kvalobs;
using namespace miutil;
using namespace boost::assign;

namespace Weather
{
  WeatherTable::WeatherTable( QWidget *parent, QString name, int type )
    : Q3Table( parent )
  {
    WeatherDialog* wd = dynamic_cast<WeatherDialog*>(parent->parent());
    
    synFlg sf;
    vector<synDat> dataList;
    vector<synFlg> flagList;
    QString pName(name);
    miutil::miTime protime("1900-01-01 00:00:00");
    
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
	cerr << "Knut tester obstime: " << dit->obstime() << endl;
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

  void WeatherTable::displayVerticalHeader( vector<miutil::miTime>& timeList) {
    int irow = 0;
    for ( vector<miutil::miTime>::iterator it = timeList.begin(); it != timeList.end(); it++) {
      miutil::miString mTime = (*it).isoTime();
      const char* cTime = mTime.c_str();
      QString dateTime;
      dateTime = QString(cTime);
      verticalHeader()->setLabel(irow,dateTime);
      ++irow;
    }
  }

  void WeatherTable::displayData(QString pName, vector<synDat>& dataList, vector<synFlg>& flagList) {
    int iRow = 0;
    vector<synFlg>::iterator fit = flagList.begin();
    for ( vector<synDat>::iterator it = dataList.begin(); it != dataList.end(); it++) {
      for ( int iCol = 0; iCol < NP; iCol++ ) {
	QString strdat;
	QString strtyp;
       	QString strflg;
       	strflg = (*fit).sflg[iCol];
	if ( (*it).sdat[iCol] < -32000 ) {
	  strdat = "";
	  strtyp = "";
	}
	else {
	  strdat = strdat.setNum((*it).sdat[iCol],'f',d1Par[iCol]);
	  strtyp = strtyp.setNum((*it).styp[iCol]);
	}
	if ( pName == "corr" ) {
	  WeatherTableItem* datItem = new WeatherTableItem(this, Q3TableItem::OnTyping,strtyp,strdat);
	  setItem(iRow,datCol[iCol],datItem);
	  if ( strflg == "fnum = 6" )
	    datItem->isModelVal = true;
	  else
	    datItem->isModelVal = false;
	}
	else if ( pName == "orig" ) {
	  WeatherTableItem* datItem = new WeatherTableItem(this, Q3TableItem::Never,strtyp,strdat);
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

      iRow++;
      fit++;
    }
   for ( int icol = 0; icol < NL; icol++ )
      adjustColumn(icol);
  }

  void WeatherTable::displayFlags(vector<synFlg>& flagList) {
    cerr << "Flaglist size 2 = " << flagList.size() << endl;
    int iRow = 0;
    for ( vector<synFlg>::iterator it = flagList.begin(); it != flagList.end(); it++) {
      for ( int iCol = 0; iCol < NP; iCol++ ) {
	QString strdat;
	strdat = (*it).sflg[iCol];
	FlagItem* flgItem = new FlagItem(this, Q3TableItem::Never,"",strdat);
	setItem(iRow,datCol[iCol],flgItem);
      }
      iRow++;
    }

    for ( int icol = 0; icol < NL; icol++ )
      adjustColumn(icol);
    for ( int icol = 0; icol < NC; icol++ )
      hideColumn(cbCol[icol]);
  }

  void WeatherTable::getModifiedData( DataConsistencyVerifier::DataSet & mod )
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
				"Kan ikke lagre",
				"Raden fins ikke i databasen",
				QMessageBox::Ok,
				QMessageBox::NoButton );
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
    float uplim = highMap[kvDat.paramID()];
    float downlim = lowMap[kvDat.paramID()];
    if ( (newCorr > uplim || newCorr < downlim) && newCorr != -32766 ) {
      QMessageBox::information( this,
				"Ulovlig verdi",
				"Verdien er utenfor fysikalske grenser",
				QMessageBox::Ok,
				QMessageBox::NoButton );
      if ( oldCorr == -32767.0 )
	tit->setText("");
      else
	tit->setText(oldCorrStr);
      return;
    }
    if ( fabs(newCorr - org ) < epsilon ) {
      if ( cif.flag(4) > 1 ) {
	cif.set(15,1);
	cif.set(4,1);
      }
      if ( cif.flag(6) == 0 ) {
	cif.set(15,1);
	if ( cif.flag(12) == 3 )
	  cif.set(12,1);
      }
      else if ( cif.flag(6) == 1 ) {
	cif.set(15,0);
	cif.set(6,3);
      }
      else if ( cif.flag(6) == 2 ) {
	cif.set(15,1);
	cif.set(6,0);
	if ( cif.flag(12) == 3 )
	  cif.set(12,1);
      }
    }
    if ( newCorr == -32766.0 ) {
      cif.set(15,10);
      const int misfl = cif.flag(6);
      if ( misfl == 0 || misfl == 1 )
	cif.set(6,misfl + 2);
    }
    else if ( cif.flag(6) == 1 || cif.flag(6) == 3 ) {
      cif.set(6,1);
      cif.set(15,5);
    }
    else if ( cif.flag(6) == 0 || cif.flag(6) == 2 ) {
      cif.set(6,0);
      cif.set(15,7);
    }

    if ( oldCorr == -32767.0 ) {
      cif.set(15,5);                      //Interpol
      int misfl;
      if ( cif.flag(6) == 0 )
	misfl = 1;
      else
	misfl  = cif.flag(6) - 2;
      cif.set(6,misfl);
    }
    kvDat.controlinfo(cif);
    kvCorrList.push_back(kvDat);
    oldNew.push_back(op);
    rowCol.push_back(rc);
    corr.oTime = kvDat.obstime();
    corr.parName = parm[datCol[col]];
    corr.oldVal = oldCorr;
    corr.newVal = newCorr;
  }

  kvData WeatherTable::getKvData( int row, int col ) {
    miutil::miTime cTime = timeList[row];
    //Find paramID in col
    bool foundRow = false;
    int cParam = params[columnIndex[col]];
    vector<kvData>::iterator kvit;
    for ( kvit = kvDatList.begin(); kvit != kvDatList.end(); kvit++) {
      if ( (*kvit).paramID() == cParam && (*kvit).obstime() == cTime && (*kvit).typeID() == sd.styp[columnIndex[col]]) {
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
		    cTime,
		    -32767,
		    cParam,
		    cTime,
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
    QString path = QString(getenv("HQCDIR"));
    if ( path.isEmpty() ) {
      cerr << "Intet environment" << endl;
      exit(1);
    }
    int par, dum;
    float low, high;
    QString limitsFile = path + "/etc/kvhqc/slimits";
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
}
