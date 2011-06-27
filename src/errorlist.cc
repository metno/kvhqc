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
/*! \file errolist.cc
 *  \brief Code for the ErrorList class.
 *
 *  Displays the error list.
 *
*/
#define NDEBUG
#include <QtGui>
#include <cassert>
//#include <qevent.h>
#include <QEvent>
#include <q3textstream.h>
#include <qcursor.h>
#include <qprinter.h>
#include <q3textedit.h>
#include <q3simplerichtext.h>
#include "errorlist.h"
#include "hqcmain.h"
#include "ErrorListFirstCol.h"
#include "BusyIndicator.h"
#include "ExtendedFunctionalityHandler.h"
#include "missingtable.h"
#include "StationInformation.h"
#include "TypeInformation.h"
#include <kvcpp/KvApp.h>

typedef StationInformation<kvservice::KvApp> StationInfo;
typedef TypeInformation<kvservice::KvApp>    TypeInfo;

using namespace kvalobs;

int mP[] = {61,81,109,110,177,178,211,262};
int cP[] = {  1,  2,  3,  4,  6,  7,  9, 10, 11, 12,
		    13, 14, 15, 17, 18, 19, 20, 21, 22, 23,
		     24, 25, 26, 27, 27, 28, 31, 32, 33, 34,
		     35, 36, 37, 38, 39, 40, 41, 42, 43, 44,
		     45, 46, 47, 48, 49,151,301,302,303,304,
		     305,306,307,308,1021,1022,1025,1026};
const int headSize = 0;

/*!
 * \brief Constructs the error list
 */
ErrorList::ErrorList(QStringList& selPar,
		     const miutil::miTime& stime,
		     const miutil::miTime& etime,
		     int noSel,
		     int noParam,
		     QWidget* parent,
		     int lity,
		     int metty,
		     int* noSelPar,
		     vector<model::KvalobsData>& dtl,
		     vector<modDatl>& mdtl,
		     list<kvStation>& slist,
		     int dateCol,
		     int ncp,
		     QString& userName)
  : Q3Table( 1000, 100, parent, "table")
  , mainWindow( getHqcMainWindow( parent ) )
{
  setVScrollBarMode( Q3ScrollView::AlwaysOn  );
  setMouseTracking(true);
  BusyIndicator busyIndicator;
  stationidCol = 1;
  typeidCol = 7;

  efh = new ExtendedFunctionalityHandler( this, this );
  setCaption("HQC - Feilliste");
  setSorting(TRUE);
  readLimits();
  setSelectionMode( Q3Table::SingleRow );

  opName = userName;
  fDlg = new FailInfo::FailDialog(this);

  connect( this, SIGNAL( clicked( int, int, int, const QPoint& ) ),
	   SLOT( tableCellClicked( int, int, int) ) );

  connect( this, SIGNAL( valueChanged( int, int ) ),
	   SLOT( markModified( int, int ) ) );

  connect( this, SIGNAL( valueChanged( int, int ) ),
	   SLOT( clearOtherMods( int, int ) ) );

  connect( this, SIGNAL( doubleClicked( int, int, int, const QPoint&) ),
	   this, SLOT( showFail(int, int, int, const QPoint&) ) );

  connect( this, SIGNAL( currentChanged(int, int) ),
	   this, SLOT  ( updateFaillist(int, int) ) );

  connect( this, SIGNAL( currentChanged(int, int) ),
	   this, SLOT  ( showSameStation()  ) );

  connect( this, SIGNAL( currentChanged(int, int) ),
    this, SLOT  ( signalStationSelected( int ) ) );
  //  connect( this, SIGNAL( stationSelected( miMessage ) ),
  //	   mainWindow, SLOT  ( signalStationSelected( miMessage ) ) );

  connect( mainWindow, SIGNAL( windowClose() ),
	   this, SIGNAL( errorListClosed() ) );

  if (!KvApp::kvApp->getKvObsPgm(obsPgmList, statList, FALSE))
    cerr << "Can't connect to obs_pgm table!" << endl;
  cerr.setf(ios::fixed);
  int antRow = 0;
  QString fTyp = "";
  setNumRows( 0 );
  setNumCols( 0 );
  setNumCols(21);
  setShowGrid(FALSE);
  verticalHeader()->hide();
  horizontalHeader()->setLabel(1, "Stnr");
  horizontalHeader()->setLabel(2, "Navn");
  horizontalHeader()->setLabel(3, "  Md");
  horizontalHeader()->setLabel(4, "  Dg");
  horizontalHeader()->setLabel(5, "  Kl");
  horizontalHeader()->setLabel(6, "Para");
  horizontalHeader()->setLabel(7, "Type");
  horizontalHeader()->setLabel(8, "Orig.d");
  horizontalHeader()->setLabel(9, "Korr.d");
  horizontalHeader()->setLabel(10, "mod.v");
  horizontalHeader()->setLabel(11, "Flagg");
  horizontalHeader()->setLabel(12, "=");
  horizontalHeader()->setLabel(13, "Fl.v");
  horizontalHeader()->setLabel(14, "Korrigert OK");
  horizontalHeader()->setLabel(15, "Original OK");
  horizontalHeader()->setLabel(16, "Interpolert");
  horizontalHeader()->setLabel(17, "Tilfordelt");
  horizontalHeader()->setLabel(18, "Korrigert");
  horizontalHeader()->setLabel(19, "Forkastet");
  horizontalHeader()->setLabel(20, "");
  QString strDat;
  int insRow = 0;
  int missCount = 0;
  int prevTime = -1;
  int prevStat = -1;
  int prevPara = -1;
  for ( int j = 0; j < selPar.count(); j++ ) {
    for ( int i = 0; i < dtl.size(); i++ ) {
      if (  dtl[i].stnr() > 99999)
        continue;
      //??
      //      if (  dtl[i].typeId(noSelPar[j]) < 0 )
      //        continue;
      //??
      if (  dtl[i].otime() < stime || dtl[i].otime() > etime )
        continue;
      if ( !specialTimeFilter( noSelPar[j], dtl[i].otime()) )
        continue;
      if ( ! typeFilter( dtl[i].stnr(), noSelPar[j], dtl[i].typeId(noSelPar[j]), dtl[i].otime()) )
        continue;

      missObs mobs;


      QString ctr = QString::fromStdString(dtl[i].controlinfo(noSelPar[j]).flagstring());
      int flg = ctr.mid(4,1).toInt(0,16);
      int tdiff = miutil::miTime::hourDiff(dtl[i].otime(),stime);
      if ( flg == 6 ) {
	mobs.oTime = dtl[i].otime();
	mobs.time = tdiff;
	mobs.parno  = noSelPar[j];
	mobs.statno = dtl[i].stnr();
	mobs.missNo = missCount;
	if ( mobs.time - prevTime != 1 || mobs.parno != prevPara || mobs.statno != prevStat  ) {
	  missCount = 0;
	}
	if ( mobs.time - prevTime == 1 && mobs.parno == prevPara && mobs.statno == prevStat && missCount > 4 ) {
	  mList.push_back(mobs);
	}
	missCount++;
	prevTime = mobs.time;
	prevStat = mobs.statno;
	prevPara = mobs.parno;
      }
    }
  }

  int  ml = 0;

  for ( int i = 0; i < dtl.size(); i++ ) {
    if (  dtl[i].stnr() > 99999) continue;
#warning Is showTypeId correct here? (It was a bug before checking if a pointer was less than zero)
    //    if (  dtl[i].showTypeId() < 0 ) continue;
    if (  dtl[i].otime() < stime || dtl[i].otime() > etime ) continue;
    mem memObs;
    memObs.obstime = dtl[i].otime();
    memObs.tbtime = dtl[i].tbtime();
    memObs.name = dtl[i].name();
    memObs.stnr = dtl[i].stnr();

    for ( int j = 0; j < selPar.count(); j++ ) {
      bool stp = specialTimeFilter( noSelPar[j], dtl[i].otime());
      if ( !stp ) continue;
      bool tp = typeFilter( dtl[i].stnr(), noSelPar[j], dtl[i].typeId(noSelPar[j]), dtl[i].otime());
      if ( !tp ) continue;
      memObs.typeId      = dtl[i].typeId(noSelPar[j]);
      memObs.orig        = dtl[i].orig(noSelPar[j]);
      memObs.corr        = dtl[i].corr(noSelPar[j]);
      memObs.sen         = dtl[i].sensor(noSelPar[j]);
      memObs.lev         = dtl[i].level(noSelPar[j]);
      memObs.controlinfo = dtl[i].controlinfo(noSelPar[j]).flagstring();
      memObs.useinfo     = dtl[i].useinfo(noSelPar[j]).flagstring();
      memObs.cfailed     = dtl[i].cfailed(noSelPar[j]);
      memObs.parNo       = noSelPar[j];
      memObs.parName     = selPar[j];
      memObs.morig = -32767.0;

      for ( int k = 0; k < mdtl.size(); k++) {
	miutil::miTime modeltime = mdtl[k].otime;
	int modelstnr = mdtl[k].stnr;
	if ( modelstnr == memObs.stnr && modeltime == memObs.obstime ) {
	  for ( int l = 0; l < NOPARAMMODEL; l++ ) {
	    if ( memObs.parNo == mP[l] )
	      memObs.morig = mdtl[k].orig[mP[l]];
	  }
	}
      }

      //Priority filters for controls and parameters
      QString flTyp = "";
      int flg = errorFilter(noSelPar[j],
			    memObs.controlinfo,
			    memObs.cfailed,
			    flTyp);

      if ( obsInMissList(memObs) ) {
	missList.push_back(memObs);
	//	if ( ml == 0 )
	ml++;
	continue;
      }
      if ( flg <= 1 && flg > -3)
	continue;
      if ( flg == -3) {
	QString qStrCtrInfo = QString::fromStdString(memObs.controlinfo);
	flg = qStrCtrInfo.mid(6,1).toInt(0,16);
      }
      memObs.flg = flg;
      memObs.flTyp = flTyp;
      //Insert data into appropriate memory stores
      if ( lity == erLi || lity == alLi ) {	
	
	if (((flg == 2 || flg == 3) && flTyp == "fr" ) ||
	    (flg == 2 && (flTyp == "fcc" || flTyp == "fcp") ) ||
	    ((flg == 2 || flg == 3 ||flg == 4 || flg == 5) && flTyp == "fnum") ||
	    ((flg == 2 || flg == 3)&& flTyp == "fw") ||
	    ((flg == 2 || flg == 4 || flg == 5 ) && flTyp == "fs" ) )
	  
	  memStore1.push_back(memObs);
	
	else if (((flg == 4 || flg == 5 || flg == 6) && flTyp == "fr" ) ||
		 ((flg == 3 || flg == 4 || flg == 6 || flg == 7 || flg == 9 || 
		   flg == 0xA || flg == 0xB || flg == 0xD ) && flTyp == "fcc" ) || 
		 ((flg == 3 || flg == 4 || flg == 6 || flg == 7 || 
		   flg == 0xA || flg == 0xB ) && flTyp == "fcp" ) || 
		 ((flg == 3 || flg == 6 || flg == 9)&& flTyp == "fs" ) ||
		 (flg == 6 && flTyp == "fnum") ||
		 (( flg == 3 || flg == 4 || flg == 6) && flTyp == "fpos") ||
		 ((flg == 2 || flg == 3) && flTyp == "ftime") ||
		 ((flg == 4 || flg == 5 || flg == 6) && flTyp == "fw") ||
		 (flg > 0 && flTyp == "fmis" ) ||
		 (flg == 7 && flTyp == "fd") )
	  
	  memStore2.push_back(memObs);
      }
      
      
      
      
      
      //      }
      else if ( lity == erSa ) {
	if ( ((flg == 4 || flg == 5 || flg == 6) && flTyp == "fr" ) ||
	     (flg == 2 && flTyp == "fs" ))
	  memStore2.push_back(memObs);
      }
    }
  }
  ////////
  cerr << "Memory store 1 size = " << memStore1.size() << "  no of params = " <<selPar.count() << endl;
  for ( int i = 0; i < memStore1.size(); i++ ) {
    cerr << setw(7) << i;
    cerr << setw(7) << memStore1[i].stnr << setw(21) << memStore1[i].obstime 
	 << setw(5) << memStore1[i].parNo << setw(5) << memStore1[i].parName.toStdString() 
	 << setw(9) << setprecision(1) << memStore1[i].orig 
	 << setw(9) << setprecision(1) << memStore1[i].corr 
         << setw(9) << setprecision(1) << memStore1[i].morig << "  " 
	 << setw(5) << memStore1[i].flTyp.toStdString() << "  " <<memStore1[i].flg << "  "
	 << memStore1[i].controlinfo << "  " <<memStore1[i].cfailed << endl;
  }
  cerr << endl;
  cerr << "Memory store 2 size = " << memStore2.size() << "  no of params = " <<selPar.count() << endl;
  for ( int i = 0; i < memStore2.size(); i++ ) {
    cerr << setw(7) << i;
    cerr << setw(7) << memStore2[i].stnr << setw(21) << memStore2[i].obstime 
	 << setw(5) << memStore2[i].parNo << setw(5) << memStore2[i].parName.toStdString() 
	 << setw(9) << setprecision(1) << memStore2[i].orig 
	 << setw(9) << setprecision(1) << memStore2[i].corr 
         << setw(9) << setprecision(1) << memStore2[i].morig << "  " 
	 << setw(5) << memStore2[i].flTyp.toStdString() << "  " <<memStore2[i].flg << "  "
	 << memStore2[i].controlinfo << "  " <<memStore2[i].cfailed << endl;
  }
  cerr << endl;

  ////////
 checkFirstMemoryStore();

  int es = error.size();
  for ( int i = 0; i < error.size(); i++ ) {
    int stnr = memStore1[error[es-1-i]].stnr;
    miutil::miTime obstime = memStore1[error[es-1-i]].obstime;
    if ( memStore2.size() > 0 ) {
      int iCount = 0;
      vector<mem>::iterator memO = memStore2.begin();
      for ( ;memO != memStore2.end(); memO++ ) {
	iCount++;
	int cStnr = memO->stnr;
	miutil::miTime cTime = memO->obstime;
	if ( (stnr > cStnr || (stnr == cStnr && obstime >= cTime)) && iCount < memStore2.size() )
	  continue;
	else {
	  if ( iCount == memStore2.size() ) {
	    memStore2.push_back(memStore1[error[es-1-i]]);
	  }
	  else {
	    memStore2.insert(memO,memStore1[error[es-1-i]]);
	  }
	  break;
	}
      }
    }
    else {
      memStore2.push_back(memStore1[error[es-1-i]]);
    }
  }

  vector<mem>::iterator memI = memStore1.end();
  memI--;
  int ri = error.size() - 1;
  int ir = memStore1.size() - 1;
  if ( ri >= 0 ) {
    for ( ;memI >= memStore1.begin(); memI-- ) {
      if ( ir == error[ri] ) {
	memStore1.erase(memI);
	ri--;
      }
      ir--;
    }
  }
  ///////
  cerr << "Memory store 1 second time size = " << memStore1.size() 
       << "  no of params = " <<selPar.count() << endl;
  for ( int i = 0; i < memStore1.size(); i++ ) {
    cerr << setw(14) << i;
    cerr << setw(7) << memStore1[i].stnr << setw(21) << memStore1[i].obstime 
	 << setw(5) << memStore1[i].parNo << setw(5) << memStore1[i].parName.toStdString() 
	 << setw(9) << setprecision(1) << memStore1[i].orig 
	 << setw(9) << setprecision(1) << memStore1[i].corr 
	 << setw(9) << setprecision(1) << memStore1[i].morig 
	 << setw(5) << memStore1[i].flTyp.toStdString() << "  " <<memStore1[i].flg << "  "
	 << memStore1[i].controlinfo << "  " <<memStore1[i].cfailed << endl;
  }
  cerr << endl;
  cerr << "Memory store 2 second time size = " << memStore2.size() 
       << "  no of params = " <<selPar.count() << endl;
  for ( int i = 0; i < memStore2.size(); i++ ) {
    cerr << setw(14) << i;
    cerr << setw(7) << memStore2[i].stnr << setw(21) << memStore2[i].obstime 
	 << setw(5) << memStore2[i].parNo << setw(5) << memStore2[i].parName.toStdString() 
	 << setw(9) << setprecision(1) << memStore2[i].orig 
	 << setw(9) << setprecision(1) << memStore2[i].corr 
	 << setw(9) << setprecision(1) << memStore2[i].morig 
	 << setw(5) << memStore2[i].flTyp.toStdString() << "  " <<memStore2[i].flg << "  "
	 << memStore2[i].controlinfo << "  " <<memStore2[i].cfailed << endl;
  }
  cerr << endl;
  ///////
  error.clear();
  noError.clear();
  //  checkSecondMemoryStore();
  ///////
  for ( int i = 0; i < memStore1.size(); i++ ) {
    mem* memStore = new mem(memStore1[i]);
    updateKvBase(memStore);
    delete memStore;
  }


  //
  //Reference stations
  //
  refs rStat;
  vector<refs> rStatList;
  int pstnr = 0;
  int ppanr = 0;
  for ( int i = 0; i < memStore1.size(); i++ ) {

    int stnr =  memStore1[i].stnr;
    int panr =  memStore1[i].parNo;
    rStat.stnr = stnr;
    rStat.parNo = panr;
    double lat, lon, olat, olon;

    if ( stnr != pstnr ) {  // If new station: find position
      std::list<kvStation>::const_iterator it=slist.begin();
      for ( ;it!=slist.end(); it++){
	if (  it->stationID() == stnr ) {
	  lat = it->lat();
	  lon = it->lon();
	  break;
	}
      }
    }
    if ( pstnr != stnr || ( pstnr == stnr && panr != ppanr )) {
      // If new station, or new parameter at same station:  loop through obs_pgm
      for(CIObsPgmList obit=obsPgmList.begin();obit!=obsPgmList.end(); obit++){
	int ostnr = obit->stationID();
	int opanr = obit->paramID();
	if ( ostnr != stnr && opanr == panr ) { //another station which has the same parameter

	  std::list<kvStation>::const_iterator it=slist.begin();
	  for ( ;it!=slist.end(); it++){  //find position of the other staTION
	    if ( it->stationID() == ostnr ) {
	      olat = it->lat();
	      olon = it->lon();
	      break;
	    }
	  }
	  rStat.dist = calcdist( olon, olat, lon, lat);// Find distance between stations
	  rStat.rstnr = ostnr;
	  if (rStatList.size() == 0 )
	    rStatList.push_back(rStat);
	  else {
	    bool ins = false;
	    for ( vector<refs>::iterator dit = rStatList.begin();
		  dit != rStatList.end(); dit++ ) {
	      if ( rStat.stnr == dit->stnr &&
		   rStat.parNo == dit->parNo &&
		   rStat.rstnr == dit->rstnr) {
		ins = true;
		break;
	      }
	      if ( rStat.stnr <= dit->stnr &&
		   rStat.dist < dit->dist &&
		   rStat.parNo < dit->parNo &&
		   rStat.rstnr != dit->rstnr) {
		rStatList.insert(dit, rStat);
		ins = true;
		break;
	      }
	    }
      	    if ( !ins )
	      rStatList.push_back(rStat);
	  }
	}
      }
    }
    pstnr = stnr;
    ppanr = panr;
  }

  setNumRows( memStore2.size() + headSize );
  for ( int i = 0; i < memStore2.size(); i++ ) {
    setRowReadOnly( insRow + headSize, false);
    setItem( insRow + headSize, 0, new ErrorListFirstCol( this, insRow + headSize ) );

    cerr << i << ": " << decodeutility::kvdataformatter::
      createString( getKvData( memStore2[i] ) ) << endl;

    if ( memStore2[i].flg <= 1 &&  memStore2[i].flg > -3)
      continue;

    if ( memStore2[i].controlinfo.substr(1,1) == "6" && memStore2[i].controlinfo.substr(7,1) == "1"  )
      continue;

    strDat = strDat.setNum(memStore2[i].stnr);
    DataCell* snIt = new DataCell(this, Q3TableItem::Never,strDat);
    setItem(insRow + headSize,1,snIt);

    strDat = memStore2[i].name.left(8);
    DataCell* naIt = new DataCell(this, Q3TableItem::Never,strDat);
    setItem(insRow + headSize,2,naIt);

    strDat = QString(memStore2[i].obstime.isoTime().cStr()).mid(5,2);
    DataCell* moIt = new DataCell(this, Q3TableItem::Never,strDat);
    setItem(insRow + headSize,3,moIt);

    strDat = QString(memStore2[i].obstime.isoTime().cStr()).mid(8,2);
    DataCell* dyIt = new DataCell(this, Q3TableItem::Never,strDat);
    setItem(insRow + headSize,4,dyIt);

    strDat = QString(memStore2[i].obstime.isoTime().cStr()).mid(11,2);
    DataCell* clIt = new DataCell(this, Q3TableItem::Never,strDat);
    setItem(insRow + headSize,5,clIt);

    strDat = memStore2[i].parName;
    DataCell* paIt = new DataCell(this, Q3TableItem::Never,strDat);
    setItem(insRow + headSize,6,paIt);

    strDat = strDat.setNum(memStore2[i].typeId);
    DataCell* tiIt = new DataCell(this, Q3TableItem::Never,strDat);
    setItem(insRow + headSize,7,tiIt);

    if ( paramIsCode(memStore2[i].parNo) == 0 )
      strDat = strDat.setNum(memStore2[i].orig,'f',0);
    else {
      strDat = strDat.setNum(memStore2[i].orig,'f',1);
    }
    DataCell* ogIt = new DataCell(this, Q3TableItem::Never,strDat);
    setItem(insRow + headSize,8,ogIt);

    if ( paramIsCode(memStore2[i].parNo) == 0 )
      strDat = strDat.setNum(memStore2[i].corr,'f',0);
    else {
      strDat = strDat.setNum(memStore2[i].corr,'f',1);
    }
    DataCell* coIt = new DataCell(this, Q3TableItem::Never,strDat);
    setItem(insRow + headSize,9,coIt);

    strDat = strDat.setNum(memStore2[i].morig,'f',paramIsCode(memStore2[i].parNo));
    DataCell* mlIt = new DataCell(this, Q3TableItem::Never,strDat);
    setItem(insRow + headSize,10,mlIt);

    strDat = memStore2[i].flTyp;
    DataCell* fiIt = new DataCell(this, Q3TableItem::Never,strDat);
    setItem(insRow + headSize,11,fiIt);

    strDat = "=";
    DataCell* eqIt = new DataCell(this, Q3TableItem::Never,strDat);
    setItem(insRow + headSize,12,eqIt);

    strDat = strDat.setNum(memStore2[i].flg);
    DataCell* fgIt = new DataCell(this, Q3TableItem::Never,strDat);
    setItem(insRow + headSize,13,fgIt);

    insRow++;
  }

  cerr << "Antall rader = " << insRow << endl;

  for (int icol = 14; icol < 16; icol++) {
    for ( int irow = headSize; irow < insRow + headSize; irow++ ) {
      OkTableItem* okIt = new OkTableItem(this, "");
      setItem(irow,icol,okIt);
    }
  }

  for (int icol = 16; icol < 19; icol++) {
    for ( int irow = headSize; irow < insRow + headSize; irow++ ) {
      CrTableItem* crIt = new CrTableItem(this, Q3TableItem::WhenCurrent, "" ,// );
					  icol != 14 and icol != 15 and icol != 19 );
      setItem(irow,icol,crIt);
    }
  }

  for ( int irow = headSize; irow < insRow + headSize; irow++ ) {
    OkTableItem* okIt = new OkTableItem(this, "");
    setItem(irow, 19, okIt);
  }

  for ( int icol = 0; icol < 14; icol++ )
    adjustColumn(icol);
  for ( int icol = 14; icol < 16; icol++ )
    setColumnWidth(icol, 90);
  for ( int icol = 16; icol < 20; icol++ )
    setColumnWidth(icol, 70);

  setColumnReadOnly ( 20, true );
  setColumnStretchable( 20, true );
  showSameStation();

  setIcon( QPixmap("/usr/local/etc/kvhqc/hqc.png") );
  setCaption("Feilliste");
}

ErrorList::~ErrorList() {
  //  delete stTT;
}

bool ErrorList::obsInMissList(mem memO) {
  for ( int i = 0; i < mList.size(); i++ ) {
    if ( mList[i].statno == memO.stnr && mList[i].parno == memO.parNo && mList[i].oTime == memO.obstime )
      return true;
  }
  return false;
}

void CrTableItem::paint( QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected ) {
  Q3TableItem::paint( p, cg, cr, selected );
  p->drawRect( 0, 0, cr.width(), cr.height());
}

const QRegExp CrTableItem::re( "(\\-?[0-9]+(\\.[0-9])?)" );
const QRegExpValidator CrTableItem::validator( CrTableItem::re, NULL );
QWidget *CrTableItem::createEditor() const
{
  QLineEdit *le = dynamic_cast<QLineEdit *>( Q3TableItem::createEditor() );
  if ( numbers )
    le->setValidator( &validator );
  return le;
}


void OkTableItem::paint( QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected ) {
  Q3CheckTableItem::paint( p, cg, cr, selected );
  p->drawRect( 0, 0, cr.width(), cr.height());
}


void DataCell::paint( QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected )
{
  p->setBrush(Qt::green);
  QColorGroup g( cg );
  g.setColor( QColorGroup::Background, Qt::green );
  Q3TableItem::paint( p, g, cr, selected );
}

bool ErrorList::priorityParameterFilter(int parNo) {
  for ( int i = 0; i < npnc; i++ ) {
    if ( parNo == parNoControl[i] ) {
      return false;
    }
  }
  if ( parNo >= 1000 )
    return false;
  else
    return true;
}

int ErrorList::priorityControlFilter(QString control) {
  QStringList allFails = QStringList::split(",",control,false);
  if ( allFails.count() > 0 ) {
    for ( int i = 0; i < npcc; i++ ) {
      if ( allFails[0] == controlNoControl[i] && allFails.count() == 1) {
	return 0;
      }
      else if ( allFails[0] == controlNoControl[i] ) {
	return -1;
      }
    }
  }
  return 1;
}

int ErrorList::errorFilter(int parNo,string ctrInfo, string cFailed, QString& flTyp) {
  QString flTypes[] = {"fagg","fr","fcc","fs","fnum","fpos","fmis","ftime","fw","fstat","fcp","fclim","fd","fpre","fcombi","fhqc"};
  QString qStrCtrInfo = QString::fromStdString(ctrInfo);
  QString control = QString::fromStdString(cFailed);
  int flg = 0;
  int maxflg = -1;
  if ( qStrCtrInfo.mid(13,1).toInt(0,10) > 0 ) 
    return maxflg;
  if ( !priorityParameterFilter(parNo) )
    return maxflg;
  if ( priorityControlFilter(control) == 0 ) {
    return maxflg;
  }
  else if ( priorityControlFilter(control) == -1 )
    maxflg = -2;
  int flInd = 0;
  for ( int i = 0; i < 16; i++ ) {
    flg = qStrCtrInfo.mid(i,1).toInt(0,16);
    if ( flg > 1  && maxflg > -2) {
      maxflg = flg;
      flTyp = flTypes[i];
      break;
    }
    else if (flg > 1 )
      maxflg = -1;
  }

  if ( flTyp == "fmis" ) {
    maxflg = -3;
  }
  flg =  qStrCtrInfo.mid(15,1).toInt(0,16);
  if ( flg > 0 )//Hqc-flag allerede satt. Ikke ny kontroll
    maxflg = -1;
  return maxflg;
}

void ErrorList::checkFirstMemoryStore() {
  int j = 0, l = 0;
  for ( int i = 0; i < memStore1.size(); i++ ) {
    kvControlInfo cif(memStore1[i].controlinfo); 
    if ( memStore1[i].flTyp == "fr" ) {
      if ( paramHasModel(memStore1[i].parNo) ) {
	if ( cif.flag(4) == 1 || (cif.flag(4) > 1 && cif.flag(8) == 1) ) {
	  noError.push_back(i);
	}
	else if ( (cif.flag(4) > 1 && cif.flag(8) > 1) || cif.flag(8) == 0 ) {
	  error.push_back(i);
	}
      }
      else {
	for ( int k = 2; k < 16; k++ ) {
	  //	  int iFlg = control.mid(k,1).toInt(0,16);
	  int iFlg = cif.flag(k);
	  if ( iFlg > 1 ) {
	    error.push_back(i);
	    break;
	  }
	}
      }
    }
    else if ( memStore1[i].flTyp == "fs" ) {
      if ( cif.flag(3) == 2 && cif.flag(10) > 1 )
	error.push_back(i);
      else if ( cif.flag(3) == 2 && cif.flag(10) <= 1 ) {
	if ( cif.flag(1) == 1 && cif.flag(8) == 1 ) {
	  noError.push_back(i);
	}
	else if ( cif.flag(1) > 1 || cif.flag(8) > 1 ) {
	  error.push_back(i);
	} 
      }
      else if ( cif.flag(3) == 4 && cif.flag(10) > 1 ) {
	error.push_back(i);
      }
      else if ( cif.flag(3) == 4 && cif.flag(10) <= 1 && cif.flag(1) <= 1 && cif.flag(8) <= 1 ) {
	noError.push_back(i);
      }
      else if ( cif.flag(3) == 5 && ( cif.flag(1) > 1 || cif.flag(8) > 1 ) ) {
	error.push_back(i);
      }
      else if ( cif.flag(3) == 5 && cif.flag(1) <= 1 &&  cif.flag(8) <= 1  ) {
	noError.push_back(i);
      }
    }
    //TODO: Proper treatment of fcc=2 and fcp=2
    /*
    else if ( memStore1[i].flTyp == "fcc" ) {
      if ( cif.flag(2) == 2 )
      // find the other parameter
	error.push_back(i);
    }
    else if ( memStore1[i].flTyp == "fcp" ) {
      if ( cif.flag(10) == 2 )
      // find the other parameter
	error.push_back(i);
    }
    */
    else if ( memStore1[i].flTyp == "fnum" ) {
      if ( memStore1[i].parNo == 177 || memStore1[i].parNo == 178 ) {
	error.push_back(i);
      }
    }
    else if ( memStore1[i].flTyp == "fw" ) {
      if ( (cif.flag(8) == 2 || cif.flag(8) == 3) && 
	   ( cif.flag(1) > 1 || cif.flag(2) > 1 || cif.flag(3) > 1 || cif.flag(10) > 1) ) {
	error.push_back(i);
      }
      else if ( (cif.flag(8) == 2 || cif.flag(8) == 3) && 
		( cif.flag(1) <= 1 && cif.flag(2) <= 1 && cif.flag(3) <= 1 && cif.flag(10) <= 1) )  {
	noError.push_back(i);
      }
    }
    else
      noError.push_back(i);
  }
}

void ErrorList::checkSecondMemoryStore() {
  int j = 0, l = 0;
  for ( int i = 0; i < memStore2.size(); i++ ) {
    kvControlInfo cif(memStore2[i].controlinfo); 
    if ( memStore2[i].flTyp == "fr" ) {
      if ( cif.flag(1) == 6 && cif.flag(7) == 1 )
	noError.push_back(i);
      else
	error.push_back(i);
    }
    else if ( memStore2[i].flTyp == "fs" ) {
      if (cif.flag(3) == 6 )
	noError.push_back(i);
      else 
	error.push_back(i);
    }
  }
}

bool ErrorList::paramHasModel(int parNo) {
  for ( int i = 0; i < 8; i++ ) {
    if ( parNo == mP[i] )
      return true;
  }
  return false;
}

int ErrorList::paramIsCode(int parNo) {
  for ( int i = 0; i < 58; i++ ) {
    if ( parNo == cP[i] )
      return 0;
  }
  return 1;
}
void ErrorList::tableCellClicked(int row,
				 int col,
				 int button) {//,
  //				 const QPoint& mousePos,
  //				 vector<model::KvalobsData>& dtl) {
  if ( col == 0 && row >= 0) {
    selectRow(row);
  }
}

double ErrorList::calcdist(double lon1, double lat1, double lon2, double lat2) {
  double alon1 = M_PI*lon1/180.0;
  double alon2 = M_PI*lon2/180.0;
  double alat1 = M_PI*lat1/180.0;
  double alat2 = M_PI*lat2/180.0;
  double dist;
  dist = 2.0*asin(sqrt((sin((alat1-alat2)/2.0))*(sin((alat1-alat2)/2.0)) +
		       cos(alat1)*cos(alat2)*(sin((alon1-alon2)/2))*(sin((alon1-alon2)/2))));
  return 6378.0*dist;
}

bool ErrorList::specialTimeFilter( int par, miutil::miTime otime) {
  bool spf = true;
  if ( ((par == 214 || par == 216) && !(otime.hour() == 6 || otime.hour() == 18)) ||
       (par == 112 && otime.hour() != 6) ){
    spf = false;
  }
  return spf;
}

bool ErrorList::typeFilter(int stnr, int par, int typeId, miutil::miTime otime) {
  //??
  //  if ( typeId  == 501 ) return false;
  //??
  bool tpf = false;
  for ( vector<currentType>::iterator it = mainWindow->currentTypeList.begin(); it != mainWindow->currentTypeList.end(); it++) {
    if ( stnr == (*it).stnr && abs(typeId) == (*it).cTypeId && par == (*it).par && otime.date() >= (*it).fDate && otime.date() <= (*it).tDate )
      tpf = true;
  }
  return tpf;
}

/*!
 * \brief Update kvalobs, set hqc-flag = 2 for obs not in errorlist
 */
void ErrorList::updateKvBase(mem* memStore)
{
  if ( mainWindow->reinserter != NULL ) {
    kvData kd = getKvData( *memStore );
    //TODO: Remove next 3 lines when the new QC1-9 is ready
    kvControlInfo cif = kd.controlinfo();
    cif.set(15,2);
    kd.controlinfo(cif);
    CKvalObs::CDataSource::Result_var result;
    {
      BusyIndicator busyIndicator;
      result = mainWindow->reinserter->insert(kd);
    }
    if ( result->res != CKvalObs::CDataSource::OK ) {
      cerr << "Could send data!" << endl
	   << "Message was:" << endl
	   << result->message << endl;
      // Handle Error!
      return;
    }
    mainWindow->setKvBaseUpdated(TRUE);
  }
}

void ErrorList::updateFaillist( int row, int col) {
  if ( row > headSize - 1 && row <  numRows() )
    fDlg->failList->newData( getKvData( row ) );
}

void ErrorList::showFail( int row, int col, int butt, const QPoint& p) {
  if ( (col > 10 && col < 14) && (row > headSize - 1 && row <  numRows()) ) {
    fDlg->show();
  }
}

void ErrorList::swapRows( int row1, int row2, bool /*swapHeader*/ ) {
  Q3Table::swapRows( row1, row2, TRUE );
}
void ErrorList::sortColumn( int col, bool ascending, bool /*wholeRows*/ ) {
  emit currentChanged( currentRow(), currentColumn() );
  Q3Table::sortColumn( col, ascending, TRUE );
  emit currentChanged( currentRow(), currentColumn() );
  clearSelection( true );
  ensureCellVisible( currentRow(), 0 );
}

void ErrorList::showSameStation()
{
  int row = currentRow();
  if ( row < 0 )
    return;
  QString station = text( row, 1 );
  for ( int i = 0; i < numRows(); i++ ) {
    ErrorListFirstCol *elfc = dynamic_cast<ErrorListFirstCol*>(item( i, 0 ));
    if ( elfc != NULL ) {
      elfc->setSameStation( text( i, 1 ) == station );
      updateCell( i, 0 );
    }
  }
}

void ErrorList::markModified( int row, int col )
{
  ErrorListFirstCol * elfc = dynamic_cast<ErrorListFirstCol*>( item( row, 0) );
  assert( elfc );
  struct mem &msItem =
    memStore2[ elfc->memStoreIndex() ];

  kvData kd = getKvData( row );
  kvControlInfo cif = kd.controlinfo();


  bool OK = true;
  float cor;
  if ( col > 15 && col < 19 )
    cor = item(row, col)->text().toFloat(&OK);
  else if ( col == 15 ) {
    cor = msItem.orig;
  }
  else if ( col == 19 )
    cor = discardedValue_;
  else
    cor = msItem.corr;

  int panr = msItem.parNo;

  float uplim = highMap[panr];
  float downlim = lowMap[panr];
  if ( ( !text(row, col).isEmpty() && (cor > uplim || cor < downlim)) && col < 19 && col > 15) {
    QMessageBox::warning( this,
			  "Ulovlig verdi",
			  "Verdien er utenfor fysikalske grenser",
			  QMessageBox::Ok,
			  Qt::NoButton );
    item( row, col )->setText("");;
    return;
  }
  int fmis = cif.flag(6);
  int fd = cif.flag(12);
  switch (col) {
  case 14:
    {
      if ( fmis == 0 ) {
	if ( fd == 2 || fd == 3 || fd > 5 ) {
	  QMessageBox::information( this,
				    "Feil kolonne",
				    "Oppsamling.\nBenytt feltet Tilfordelt.",
				    QMessageBox::Ok,
				    Qt::NoButton );
	  OkTableItem* okIt = static_cast<OkTableItem*>(item( row, col));
	  okIt->setChecked(false);
	  item( row, col )->setText("");
	  updateCell(row, col);
	  return;
	}
      }
      else if ( fmis == 1 ) {
	if ( fd == 2 || fd > 5 ) {
	  QMessageBox::information( this,
				    "Feil kolonne",
				    "Oppsamling.\nBenytt feltet Tilfordelt.",
				    QMessageBox::Ok,
				    Qt::NoButton );
	  OkTableItem* okIt = static_cast<OkTableItem*>(item( row, col));
	  okIt->setChecked(false);
	  item( row, col )->setText("");
	  updateCell(row, col);
	  return;
	}
      }
      else if ( fmis == 2 ) {
	QMessageBox::information( this,
				  "Feil kolonne",
				  "Korrigert mangler.\nBenytt feltet Original OK eller Forkastet.",
				  QMessageBox::Ok,
				  Qt::NoButton );
	OkTableItem* okIt = static_cast<OkTableItem*>(item( row, col));
	okIt->setChecked(false);
	item( row, col )->setText("");
	updateCell(row, col);
	return;
      }
      else if ( fmis == 3 ) {
	QMessageBox::information( this,
				  "Feil kolonne",
				  "B�de original og Korrigert mangler.\nBenytt feltet Interpolert.",
				  QMessageBox::Ok,
				  Qt::NoButton );
	OkTableItem* okIt = static_cast<OkTableItem*>(item( row, col));
	okIt->setChecked(false);
	item( row, col )->setText("");
       	updateCell(row, col);
	return;
      }
    }
    break;
  case 15:
    {
      if ( fmis == 1 ) {
	int dsc = QMessageBox::information( this,
					    "Feil kolonne",
					    "�nsker du � sette inn -32767 som korrigert verdi?\n",
					    "Ja",
					    "Nei" );
	if ( dsc == 1 ) {
	  QMessageBox::information( this,
				    "Feil kolonne",
				    "Benytt feltet Interpolert hvis du �nsker ny interpolert verdi,\neller Korrigert OK hvis du �nsker � godkjenne eksisterende verdi",
				    QMessageBox::Ok,
				    Qt::NoButton );
	  OkTableItem* okIt = static_cast<OkTableItem*>(item( row, col));
	  okIt->setChecked(false);
	  item( row, col )->setText("");
	  updateCell(row, col);
	  return;
	}
      }
      else if ( fmis == 3 ) {
	OkTableItem* okIt = static_cast<OkTableItem*>(item( row, col));
	okIt->setChecked(false);
	item( row, col )->setText("");
	updateCell(row, col);
	QMessageBox::information( this,
				  "Feil kolonne",
				  "B�de original og korrigert mangler.\nBenytt feltet Interpolert.",
				  QMessageBox::Ok,
				  Qt::NoButton );
	return;
      }
    }
    break;
  case 16:
    {
      if ( fd > 1 ) {
	QMessageBox::information( this,
				  "Feil kolonne",
				  "Oppsamling.\nBenytt feltet Tilfordelt.",
				  QMessageBox::Ok,
				  Qt::NoButton );
	item( row, col )->setText("");
	return;
      }
    }
    break;
  case 17:
    break;
  case 18:
    {
      if ( fd >= 2 ) {
	QMessageBox::information( this,
				  "Feil kolonne",
				  "Oppsamling.\nBenytt feltet Tilfordelt.",
				  QMessageBox::Ok,
				  Qt::NoButton );
	OkTableItem* okIt = static_cast<OkTableItem*>(item( row, col));
	item( row, col )->setText("");
	//	updateCell(row, col);
	return;
      }
    }
    break;
  case 19:
    {
      if ( fmis == 1 ) {
	QMessageBox::information( this,
				  "Feil kolonne",
				  "Kan ikke forkaste.\nBenytt feltet Original OK.",
				  QMessageBox::Ok,
				  Qt::NoButton );
	OkTableItem* okIt = static_cast<OkTableItem*>(item( row, col));
	okIt->setChecked(false);
	item( row, col )->setText("");
	updateCell(row, col);
	return;
      }
      else if ( fmis == 3 ) {
	QMessageBox::information( this,
				  "Feil kolonne",
				  "Kan ikke forkaste.\nBenytt feltet Interpolert.",
				  QMessageBox::Ok,
				  Qt::NoButton );
	OkTableItem* okIt = static_cast<OkTableItem*>(item( row, col));
	okIt->setChecked(false);
	item( row, col )->setText("");
	updateCell(row, col);
	return;
      }
    }
    break;
  default:
    break;
  }
  //  cerr << "markModified( " << elfc->memStoreIndex() << ");\n";
  // Test starter
  bool emptyRec;
  if ( col == 14 || col == 15 || col == 19 ) {
    OkTableItem* okIt = static_cast<OkTableItem*>(item( row, col));
    emptyRec = !okIt->isChecked();
  }
  else
    emptyRec = text(row, col).isEmpty();
  if ( emptyRec )
    modifiedRows.erase( elfc );
  else
    //Test slutter
    modifiedRows.insert( elfc );
}

void ErrorList::clearOtherMods( int row, int col )
{
  cerr << "clearOtherMods( " << row << ", " << col << ")\n";

  if ( col > 13 && col < 20 ) {
    if ( (col > 15 && col < 19 && text( row, col ).stripWhiteSpace().isEmpty()) ) {
      item( row, col )->setText("");;
      return;
    }

    // Check if another column contains data
    for ( int icol = 14; icol < 16; icol++ ) {
      if ( icol != col ) {
	OkTableItem* okIt = static_cast<OkTableItem*>(item( row, icol));
	okIt->setChecked(false);
      	updateCell(row, icol);
      }
    }

    for ( int icol = 16; icol < 19; icol++ ) {
      if ( icol != col ) {
	if ( not text(row, icol).isEmpty() ) {
	  item( row, icol )->setText("");

	  updateCell(row, icol);
	}
      }
    }
    if ( col != 19 ) {
      OkTableItem* okIt = static_cast<OkTableItem*>(item( row, 19));
      okIt->setChecked(false);
      updateCell(row, 19);
    }
  }
}

void ErrorList::signalStationSelected( int row )
{
  if ( row < 0 )
    return;

  static int oldRow = -1;
  if ( row == oldRow )
    return;
  oldRow = row;


  cerr << "getMem(" << row << ");" << endl;
  const struct mem * m = getMem( row );
  ((HqcMainWindow*)getHqcMainWindow( this ))->sendObservations(m->obstime,true);
  ((HqcMainWindow*)getHqcMainWindow( this ))->sendStation(m->stnr);
  miMessage letter;
  letter.command = qmstrings::station;
  miutil::miString stationstr(m->stnr);
  miutil::miString otime(m->obstime.isoTime());
  letter.common = stationstr ;
  letter.common.append(",");
  letter.common.append(otime);
  emit statSel( letter );

}

void execMissingList( ErrorList* el )
{
  BusyIndicator busy;
  if ( el->mList.size() > 0 ) {
    MissingTable* mt = new MissingTable(el, el);
    mt->show();
  }
  else {
    QString missText = "Mangellisten inneholder ikke fler \nelementer enn de som vises i feillisten";
    int mb = QMessageBox::information(el,
				      "Mangelliste",
				      missText,
				      "OK");
  }
}

void ErrorList::setupMissingList( int row, int col )
{
  const struct mem *m = getMem( row );
  if ( m and m->controlinfo[4] == '6' ) {
    efh->reset( row, execMissingList, (Qt::Key) (Qt::Key_M ),
	       "Ctrl+M viser mangelliste." );
  }
  else
    efh->clear();
}

const struct ErrorList::mem *ErrorList::getMem( int row ) const
{
  ErrorListFirstCol *elfc =
    dynamic_cast<ErrorListFirstCol*>( item( row, 0) );
  if ( elfc == NULL )
    return NULL;
  return &memStore2[ elfc->memStoreIndex() ];
}

kvData
ErrorList::getKvData( const struct ErrorList::mem &m ) const
{
  return kvData( m.stnr, m.obstime, m.orig, m.parNo, m.tbtime,
		 m.typeId, m.sen, m.lev, m.corr, m.controlinfo,
		 m.useinfo, m.cfailed );
}

kvData
ErrorList::getKvData( int row ) const
{
  const struct mem *m = getMem( row );
  if ( !m )
    return kvData();
  return getKvData( *m );
 }

typedef list<kvData> kvDataList;

void ErrorList::saveChanges()
{
  cerr << "saving changes" << endl;

  DataReinserter<kvservice::KvApp> *reinserter = mainWindow->reinserter;
  if ( ! reinserter ) {
    QMessageBox::critical( this,
			   "Ikke autentisert",
			   "Du er ikke autentisert som operat�r.\n"
			   "Kan ikke lagre data.",
			   QMessageBox::Ok,
			   Qt::NoButton );
    return;
  }

  if ( modifiedRows.empty() ) {
    QMessageBox::information( this,
			      "Ingen ulagret data.",
			      "Det fins ingen ulagrede data",
			   QMessageBox::Ok,
			   Qt::NoButton );
    return;
  }

  kvDataList modData;

  for ( CIModList it = modifiedRows.begin(); it != modifiedRows.end(); it++ ) {
    int row = (*it)->row();

    int ccol = 0;

    for ( int icol = 14; icol < 16; icol++ ) {
      OkTableItem* okIt = static_cast<OkTableItem*>(item( row, icol));
      if ( okIt->isChecked() ) {
	ccol = icol;
	break;
      }
    }

    for ( int icol = 16; icol < 19; icol++ ) {
      if ( not text(row, icol).isEmpty() ) {
	ccol = icol;
	break;
      }
    }

    OkTableItem* okIt = static_cast<OkTableItem*>(item( row, 19));
    if ( okIt->isChecked() )
      ccol = 19;

    kvData kd = getKvData( row );
    kvControlInfo cif = kd.controlinfo();
    kvUseInfo uif = kd.useinfo();
    cerr << "Gamle flagg = " << cif << " " << uif << endl;

    int fmis = cif.flag(6);
    int fd = cif.flag(12);
    switch ( ccol ) {
    case 14:
      {
	if ( kd.original() == kd.corrected() && ( fd == 0 || fd == 1 || fd == 3 ) && fmis < 2 ) {
	  cif.set(15,1);
	  if ( fd == 3 )
	    cif.set(12,1);
	}
	else {
	  if ( fmis == 0 ) {
	    cif.set(15,7);
	  }
	  else if ( fmis == 1 ) {
	    cif.set(15,5);
	  }
	}
      }
      break;
    case 15:
      {
	if ( cif.flag(4) > 1 ) {
	  cif.set(15,1);
	}
	if ( fmis == 0 ) {
	  cif.set(15,1);
	  kd.corrected(kd.original());
	  if ( cif.flag(12) == 3 )
	    cif.set(12,1);
	}
	else if ( fmis == 1 ) {
	  cif.set(15,0);
	  cif.set(6,3);
	  kd.corrected(kd.original());
	}
	else if ( fmis == 2 ) {
	  cif.set(15,1);
	  cif.set(6,0);
	  kd.corrected(kd.original());
	  if ( cif.flag(12) == 3 )
	    cif.set(12,1);
	}
	else if ( fmis == 3 ) {
	  cerr << "Vi skulle ikke v�rt her" << endl;
	}
      }
      break;
    case 16:
      {
	if ( fmis == 0 || fmis == 2 ) {  //original exists, this is a correction 
	  //	  cif.set(6,0);
	  cif.set(6,4);
	  cif.set(15,7);
	}
	else if ( fmis == 1 || fmis == 3 ) {  //original is missing, this is an interpolation
	  cif.set(6,1);
	  cif.set(15,5);
	}
      }
      break;
    case 17:
      {
	if ( fmis == 0 || fmis == 2 ) {
	  //	if ( fmis == 0 ) {
	  cif.set(6,4);
	}
	else if ( fmis == 1 || fmis == 3 ) {
	  cif.set(6,1);
	}
	cif.set(12,2);
	cif.set(15,6);
      }
      break;
    case 18:
      {
	if ( fmis == 0 || fmis == 2 || fmis == 4 ) {  //original exists, this is a correction
	  //	  cif.set(6,0);
	  cif.set(6,4);
	  cif.set(15,7);
	}
	else if ( fmis == 1 || fmis == 3 ) {  //original is missing, this is an interpolation
	  cif.set(6,1);
	  cif.set(15,5);
	}
      }
      break;
    case 19:
      {
	int fmis = cif.flag(6);
	if ( fmis == 1 )
	  cerr << "VI SKULLE IKKE V�RT HER!!!" << endl;
	else if ( fmis == 3 )
	  cerr << "VI SKULLE IKKE V�RT HER!!!" << endl;
	else {
	  cif.set(15,10);
	  if ( fmis == 0 )
	    cif.set(6,2);
	}
      }
      break;
    case 0:
      break;
    default:
      // Undo changes:
      cif.set(15,0); break;
    }
    kd.controlinfo( cif );
    cerr << "Nye flagg    = " << cif << " " << uif << endl;

    //    if ( ccol > 14 and ccol < 19 )
    if ( ccol > 15 and ccol < 19 )
      kd.corrected( text( row, ccol ).toFloat() );
    else if ( ccol == 15 ) {
      const int tableOriginalValuePos = 8;
      float newCorrected = text( row, tableOriginalValuePos ).toFloat();
      kd.corrected( kd.original() );
    }
    else if ( ccol == 19 ) {
      kd.corrected( discardedValue_ );
    }
    else if ( ccol == 0 ) {
      const int tableOriginalValuePos = 9;
      float newCorrected = text( row, tableOriginalValuePos ).toFloat();
      kd.corrected( text( row, tableOriginalValuePos ).toFloat() );
    }

    mainWindow->setKvBaseUpdated(TRUE);

    modData.push_back( kd );
  }

  cerr << decodeutility::kvdataformatter::createString( modData ) << endl;


  CKvalObs::CDataSource::Result_var result;
  {
    BusyIndicator busyIndicator;
    result = reinserter->insert( modData );
  }

  if ( result->res != CKvalObs::CDataSource::OK ) {
    QMessageBox::critical( this,
			    "Kan ikke lagre data",
			   QString( "Kan ikke lagre data!\n"
				    "Meldingen fra Kvalobs var:\n" ) +
			   QString(result->message),
			   QMessageBox::Ok,
			   Qt::NoButton );
    return;
  }

  QString message = QString::number( modifiedRows.size() )
    + " rader ble lagret til kvalobs.";
  QMessageBox::information( this,
			    "Data lagret",
			    message,
			    QMessageBox::Ok,
			    Qt::NoButton );

  modifiedRows.clear();
}

void ErrorList::readLimits() {
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
    cerr << "kan ikke �pne " << limitsFile.toStdString() << endl;
    exit(1);
  }
  Q3TextStream limitStream(&limits);
  while ( limitStream.atEnd() == 0 ) {
    limitStream >> par >> dum >> low >> high;
    lowMap[par] = low;
    highMap[par] = high;
  }
}

bool ErrorList::maybeSave()
{
  bool ret = true;
  if ( not modifiedRows.empty() ) {
    int result =
      QMessageBox::warning( this, "HQC",
			    "Du har ulagrede endringer i feillista.\n"
			    "Vil du lagre dem?",
			    "&Ja", "&Nei", "&Avbryt",
			    0, 2 );
    if ( ! result )
      saveChanges();
    ret = result != 2;
  }
  return ret;
}

bool ErrorList::event(QEvent *event)
{
  if (event->type() == QEvent::ToolTip) {
    QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
    QPoint cp = mapFromGlobal(helpEvent->globalPos());
    int cy = contentsY();
    int row = rowAt( cp.y() + cy )-1;
    int col = columnAt( cp.x() );

    QString cellText = text( row, stationidCol );
    if ( cellText.isNull() )
      return false;

    bool ok = true;
    QString tipString =
      StationInfo::getInstance(KvApp::kvApp)->getInfo( cellText.toInt( &ok ) );
    if ( !ok ) {// Cold not convert cell contents to int.
      return false;
    }

    cellText = text( row, typeidCol );
    if ( cellText.isNull() )
      return false;
    ok = true;
    tipString += " - " + TypeInfo::getInstance(KvApp::kvApp)->getInfo( cellText.toInt( &ok ) );
    if ( !ok ) { // Cold not convert cell contents to int.
      return false;
    }
    QToolTip::showText(helpEvent->globalPos(), tipString);
  }
  return QWidget::event(event);
}

void ErrorList::closeEvent( QCloseEvent * event )
{
  if ( maybeSave() )
    Q3Table::closeEvent(event);
  else
    event->ignore();
}


QString DataCell::key() const {
  QString item;
  if ( col() == 1 || col() == 3 || col() == 5 || col() == 7 || col() == 11) {
    item.sprintf("%08d",text().toInt());
  }
  else if ( col() == 8 || col() == 9 || col() == 10 ) {
    item.sprintf("%08.1f",text().toDouble()+33000);
  }
  else {
    item = text();
  }
  return item;
}

bool ErrorList::isCoastStation(int stnr) {
  for ( int ic = 0; ic < mainWindow->coastStations.size(); ic++ ) {
    if ( stnr ==mainWindow->coastStations[ic] )
      return true;
  }
  return false;
}

double ErrorList::FF() {
  for ( int i = 0; i < memStore2.size(); i++ ) {
    if ( memStore2[i].parNo == 81 )
      return memStore2[i].orig;
  }
  return 0.0;
}
