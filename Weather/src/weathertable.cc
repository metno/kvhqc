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
#include <KvApp.h>
#include <kvalobs/kvDataOperations.h>
#include <kvalobs/kvModelData.h>
#include <sstream>
#include <qapplication.h>
#include <qmessagebox.h>
#include <qstatusbar.h>
#include <boost/assign/std/vector.hpp>

//#define NDEBUG
#include <cassert>

#include <iostream>

using namespace std;
using namespace kvalobs;
using namespace miutil;
using namespace boost::assign;
//using namespace Weather::cell;

namespace Weather
{
  /*
  void WeatherTable::setup()
  {
    setupTable();

    //connect( this, SIGNAL(valueChanged(int,int)), SLOT(markModified(int,int)));
    connect( this, SIGNAL(valueChanged(int,int)), SLOT(updateStatusbar(int,int)));
    connect( this, SIGNAL(currentChanged(int,int)), SLOT(updateStatusbar(int,int)));
  }
  */

  //  WeatherTable::WeatherTable(QToolTipGroup* ttGroup, QWidget *parent, int type )
    WeatherTable::WeatherTable( QWidget *parent, int type )
    : QTable( parent )
  {
    
    WeatherDialog* wd = dynamic_cast<WeatherDialog*>(parent->parent());
    WeatherDialog::SynObsList sList = wd->synObsList;
    vector<miutil::miTime> timeList;

    //    typedef synDat sData;
    //    typedef synFlg sFlag;

    synDat sd;
    synFlg sf;
    vector<synDat> dataList;
    vector<synFlg> flagList;
    QString pName(parent->name());
    miutil::miTime protime("1900-01-01 00:00:00");

    //    connect( this, SIGNAL(valueChanged(int,int)), SLOT(markModified(int,int)));
    connect( this, SIGNAL(valueChanged(int,int)), SLOT(updateStatusbar(int,int)));
    //    connect( this, SIGNAL(valueChanged(int,int)), SLOT(updateKvBase(int,int)));
    connect( this, SIGNAL(currentChanged(int,int)), SLOT(updateStatusbar(int,int)));
    int itest = 0, jtest = 0;
    for ( WeatherDialog::SynObsList::iterator it = wd->synObsList.begin(); 
	  it != wd->synObsList.end(); it++) {
      if ( type == 0 || (type != 0 && (*it).otime > protime) ) {
	//      if ( (*it).otime > protime ) {
	timeList.push_back((*it).otime);
	for ( int i = 0; i < NP; i++ ) {
	  if ( pName == "corr" ) {
	    sd.sdat[i] = (*it).corr[i];
	    sd.styp[i] = (*it).typeId[i];
	  }
	  else if ( pName == "orig" ) {
	    sd.sdat[i] = (*it).orig[i];
	    sd.styp[i] = (*it).typeId[i];
	  }
	  //	  else if ( pName == "flag" ) 
	  sf.sflg[i] = flagText((*it).controlinfo[i]);
	}
	//	if ( pName == "corr" || pName == "orig" )
	dataList.push_back(sd);
	//	else if ( pName == "flag" )
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
    if ( pName == "corr" || pName == "orig" ) 
      displayData(pName, dataList, flagList);
    else if ( pName == "flag" )
      displayFlags(flagList);
    toolTip = new WeatherTableToolTip( this );
    
    BusyIndicator busy;
    //    observations = getTimeObs( getStation(), getDateRange().first, getDateRange().second );
  }

  QString WeatherTable::flagText(const string& controlInfo)
  {
    QString flTypes[] = {"fqclevel","fr","fcc","fs","fnum","fpos","fmis","ftime",
			 "fw","fstat","fcp","fclim","fd","fpre","fcombi","fhqc"};
    QString flTyp;
    QString flText;
    QString ctrInfo(controlInfo);
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
    //    for ( int icol = 0; icol < numCols; icol++ ) {
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
	if ( (*it).sdat[iCol] < -32000 ) {
	  strdat = "";
	  strtyp = "";
	}
	else {
	  strdat = strdat.setNum((*it).sdat[iCol],'f',d1Par[iCol]);
	  strtyp = strtyp.setNum((*it).styp[iCol]);
	}
	if ( pName == "corr" ) {
	  WeatherTableItem* datItem = new WeatherTableItem(this, QTableItem::OnTyping,strtyp,strdat);
	  setItem(iRow,datCol[iCol],datItem);
	}
	else if ( pName == "orig" ) {
	  WeatherTableItem* datItem = new WeatherTableItem(this, QTableItem::Never,strtyp,strdat);
	  setItem(iRow,datCol[iCol],datItem);
	}
      }
      
      for ( int iCol = 0; iCol < NC; iCol++ ) {
	//	QCheckTableItem* ctItem = new QCheckTableItem(this, "");
       	QString strflg;
	//       	strflg = (*fit).sflg[dbCol[iCol]];
       	strflg = strflg.setNum((*it).styp[dbCol[iCol]]);
	if ( dbCol[iCol] < 3 ) {
	  TnCheckTableItem* ctItem = new TnCheckTableItem(this, strflg);
	  setItem(iRow,cbCol[iCol],ctItem);
	}
	else {
	  FdCheckTableItem* ctItem = new FdCheckTableItem(this, strflg);
	  setItem(iRow,cbCol[iCol],ctItem);
	}
	//	if ( pName == "orig" )
	//	  ctItem->setEnabled(false);
	//	  hideColumn(cbCol[iCol]);
      }
      
      iRow++; 
      fit++;
    }
    cerr << "Flaglist size 1 = " << flagList.size() << endl;
    /*    
    iRow = 0;
    for ( vector<synFlg>::iterator fit = flagList.begin(); fit != flagList.end(); fit++) {
      for ( int iCol = 0; iCol < NC; iCol++ ) {
	//	QCheckTableItem* ctItem = new QCheckTableItem(this, "");
      	QString strflg;
       	strflg = (*fit).sflg[dbCol[iCol]];
       	FdCheckTableItem* ctItem = new FdCheckTableItem(this, strflg);
	setItem(iRow,cbCol[iCol],ctItem);
	if ( pName == "orig" )
	  ctItem->setEnabled(false);
	//	  hideColumn(cbCol[iCol]);
      }
      iRow++; 
    }
    */
    //    for ( int icol = 0; icol < numCols; icol++ )
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
	FlagItem* flgItem = new FlagItem(this, QTableItem::Never,"",strdat);
	setItem(iRow,datCol[iCol],flgItem);
      }
      iRow++; 
    }
    //    for ( int icol = 0; icol < numCols; icol++ )
    for ( int icol = 0; icol < NL; icol++ )
      adjustColumn(icol);  
    for ( int icol = 0; icol < NC; icol++ ) 
      hideColumn(cbCol[icol]);
  }

  void WeatherTable::getModifiedData( DataConsistencyVerifier::DataSet & mod )
  {
    const int row_ = currentRow();
    const int col_ = currentColumn();
    if ( row_ >= 0 and col_ >= 0 )
      endEdit( row_, col_, true, false );

    for ( int r = 0; r < numRows(); ++ r )
    {
      for ( int c = 0; c < numCols(); ++ c )
      {
	DataConsistencyVerifier * dcv = dynamic_cast<DataConsistencyVerifier *>( item( r, c ) );
        if ( dcv ) {
          dcv->getUpdatedList( mod );
	}
      }
    }
  }

  void WeatherTable::updateStatusbar( int row, int col )
  {
    QTableItem *i = item( row, col );
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
    //    dynamic_cast<QStatusBar *>( ttGroup->parent() )->message( msg );
  }

  /*  
  void WeatherTable::polish()
  {
    QTable::polish();
    displayHorizontalHeader(numCols);
    displayVerticalHeader(timeList);
    if ( pName == "corr" || pName == "orig" ) 
      displayData(dataList, numCols);
    else if ( pName == "flag" )
      displayFlags(flagList, numCols);
    //    displayData();
    toolTip = new WeatherTableToolTip( this, ttGroup );
  }
  */
  WeatherTable::~WeatherTable( )
  {}
}
