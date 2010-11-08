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
/*! \file datatable.cc
 *  \brief Code for the DataTable class.
 *  
 *  Displays the datatable.
 *
*/

#define NDEBUG
#include <cassert>
#include <qevent.h>
#include <q3textstream.h>
#include <qcursor.h>
#include <qprinter.h>
#include <q3textedit.h>
#include <q3simplerichtext.h>
//Added by qt3to4:
#include <QPixmap>
#include "../sorttime.xpm"
#include "datatable.h"
#include "hqcmain.h"
#include "ErrorListFirstCol.h"
#include "BusyIndicator.h"
#include "ExtendedFunctionalityHandler.h"
#include "missingtable.h"


int modPar[] = {61,81,109,110,177,178,211,262};
int codeParam[] = {  1,  2,  3,  4,  6,  7,  9, 10, 11, 12,
		    13, 14, 15, 17, 18, 19, 20, 21, 22, 23,
		     24, 25, 26, 27, 27, 28, 31, 32, 33, 34,
		     35, 36, 37, 38, 39, 40, 41, 42, 43, 44,
		     45, 46, 47, 48, 49,151,273,301,302,303,
		     304,305,306,307,308,1021,1022,1025,1026};

DataTable::DataTable(QStringList selPar, 
		     int noSel, 
		     int noSelPar,
		     int* selParNo,
		     int noParam, 
		     QWidget* parent, 
		     listType lity, 
		     mettType metty,  
		     int dateCol, 
		     int ncp,
		     bool isShTy)
  : Q3Table( 1000, 100, parent, "table" ) {
  HqcMainWindow * hmw = getHqcMainWindow( parent );
  BusyIndicator busyIndicator();

  for ( int iPar = 0; iPar < NOPARAMALL; iPar++ )
    parNo[iPar] = selParNo[iPar];
  setSelectionMode(Q3Table::SingleRow);
  connect( hmw, SIGNAL( statTimeReceived(QString&)), 
  	   SLOT( focusTable(QString&)) );
  mety = metty;
  int noColPar = 4;
  hmw->nuroprpar = noSelPar == 0 ? noParam : noSelPar;
  hmw->nucoprpar = noColPar;
  timeSort = FALSE;
  QPixmap icon_sorttime(sorttime);
  Q3ToolBar* sortTool = new Q3ToolBar("Sort", hmw, this);
  QToolButton* sortButton;
  sortButton = new QToolButton( icon_sorttime, 
				tr("Sorter"), 
				"", 
				this, 
				SLOT(toggleSort()), 
				sortTool );

  connect( verticalHeader(), SIGNAL( clicked( int ) ),
	   parent, SLOT( headerClicked( int ) ) );
  setFocusStyle(Q3Table::FollowStyle);

  if ( dateCol == 0 ) {
    sortTool->setFixedSize(192,41);
  }
  else if ( dateCol == 1 ) {
    sortTool->setFixedSize(324,41);
  }
  else if ( dateCol == 2 ) {
    sortTool->setFixedSize(390,41);
  }
  else if ( dateCol == 3 ) {
    sortTool->setFixedSize(529,41);
  }

  setSorting( TRUE );
  setCaption("HQC - Dataliste");
  setNumRows(0);
  setNumCols(0);
  setNumRows(noSel + 1);
  setNumCols(noColPar*hmw->nuroprpar + 3);
  //
  // TABLE HEADING
  //
  int i = 0;
  for ( QStringList::Iterator it = selPar.begin();
	it != selPar.end(); it++) {
    QString lbl = tr( *it ) + "\nOrig";
    horizontalHeader()->setLabel( 0 + noColPar*i, lbl );

    horizontalHeader()->setLabel( 1 + noColPar*i, tr( *it ) + tr( "\nFlag" ) );
    
    horizontalHeader()->setLabel( 2 + noColPar*i, tr( *it ) + tr("\nKorr") );
    
    horizontalHeader()->setLabel( 3 + noColPar*i, tr( *it ) + tr("\nModel") );
    i++;
  }
  horizontalHeader()->setLabel( 0 + noColPar*i, tr("Stnr") );
  horizontalHeader()->setLabel( 1 + noColPar*i, tr("Obstime") );
  horizontalHeader()->setLabel( 2 + noColPar*i, tr("SynopNr.") );
  bool RR01inList = false;
  for ( int itst = 0; itst < noSelPar; itst++ ) {
    if ( selParNo[itst] == 105 ) {
      RR01inList = true;
      break;
    }
  }

  for ( int icol = 0; icol < noColPar*noParam; icol++ ) {
    setColumnWidth(icol,55);
    //    adjustColumn(icol);  
  }
  //
  // Table contents
  //
  int pistnr = 0;
  QString name, lat, lon, hoh, env;
  for ( int dt = 0; dt < noSel; dt++ ) {
    bool noErr = true;
    int istnr;
    miutil::miTime otime;
    int typeId[NOPARAM];
    int showTypeId;
    int typeIdChanged;
    double orig[NOPARAM];
    int flag[NOPARAM];
    double corr[NOPARAM];
    double morig[NOPARAM];
    string controlinfo[NOPARAM];
    string useinfo[NOPARAM];
    string cfailed[NOPARAM];
    hmw->listData(dt,
		  istnr,
		  otime, 
		  orig, 
		  flag,
		  corr,
		  morig,
		  controlinfo,
		  useinfo,
		  cfailed, 
		  typeId,
		  showTypeId,
		  typeIdChanged);
    QString strStnr;
    strStnr = strStnr.setNum(istnr);
    if ( istnr < 10 )
      strStnr = "     " + strStnr;
    else if ( istnr < 100 )
      strStnr = "    " + strStnr;
    else if ( istnr < 1000 )
      strStnr = "   " + strStnr;
    else if ( istnr < 10000 )
      strStnr = "  " + strStnr;
    else if ( istnr < 100000 )
      strStnr = " " + strStnr;
    QString strTypeId;
    if ( isShTy ) {
      strTypeId = strTypeId.setNum(showTypeId);
      //      if ( abs(showTypeId) < 10 )
      //	strTypeId = "  " + strTypeId;
      strTypeId = strTypeId.rightJustified(5,' ');
      cerr << "  :" << strTypeId.toStdString() << endl;
    }
    else
      strTypeId = "     ";
    double dlat;
    double dlon;
    double dhoh;
    int denv;
    int dsnr;

    cerr.flags(ios::fixed);

    if ( pistnr != istnr )
      hmw->findStationInfo(istnr, name, dlat, dlon, dhoh, dsnr, denv); 
    QString strSynNo;
    strSynNo = strSynNo.setNum(dsnr);
    if ( strSynNo == "0" ) strSynNo = "    ";
    lat = lat.setNum(dlat,'f',2);
    lon = lon.setNum(dlon,'f',2);
    hoh = hoh.setNum(dhoh,'f',0);
    env = env.setNum(denv);
    if ( dlon < 10.0 & dlon >= 0.0)
      lon = " " + lon;
    if ( dlat < 10.0 & dlat >= 0.0)
      lat = " " + lat;
    if ( dhoh < 10 )
      hoh = "   " + hoh;
    else if ( dhoh < 100 )
      hoh = "  " + hoh;
    else if ( dhoh < 1000 )
      hoh = " " + hoh;

    miutil::miString mTime = otime.isoTime();
    const char* cTime = mTime.c_str();
    QString strTime;
    strTime = QString(cTime);
    strTime.truncate(16);
    name = name.stripWhiteSpace();
    name = name.leftJustify(30);
    QFont vhFont("Courier", 12, QFont::Bold);
    vhFont.setStretch(QFont::ExtraCondensed);
    verticalHeader()->setFont(vhFont);
    if ( dateCol == 0 ) {
      verticalHeader()->setLabel(dt,
				 strStnr + 
				 "   " + 
				 strTime + 
				 " " + 
				 strSynNo +
				 " " +
				 strTypeId);
    }
    else if ( dateCol == 1 ) {
      verticalHeader()->setLabel(dt,
				 strStnr + 
				 "  " + 
				 lat + 
				 "   " + 
				 lon + 
				 "   " + 
				 hoh + 
				 "   " + 
				 strTime + 
				 " " + 
				 strSynNo +
				 " " +
				 strTypeId);
    }
    else if ( dateCol == 2 ) {
      verticalHeader()->setLabel(dt,
				 strStnr + 
				 "   " + 
				 name + 
				 "   " + 
				 strTime + 
				 " " + 
				 strSynNo +
				 " " +
				 strTypeId);
    }
    else if ( dateCol == 3 ) {
      verticalHeader()->setLabel(dt,
				 strStnr + 
				 "   " + 
				 name + 
				 "   " + 
				 lat + 
				 "   " + 
				 lon + 
				 "   " + 
				 hoh + 
				 "   " + 
				 strTime + 
				 " " + 
				 strSynNo+ 
				 " " +
				 strTypeId);
    }
    
    for ( int ii = 0; ii < hmw->nuroprpar; ii++) {
      int iflag = flag[selParNo[ii]];
      int if1 = iflag/10000;
      int if2 = (iflag%10000)/1000;
      int if3 = (iflag%1000)/100;
      if ( if1 > 1 || if2 > 1 || if3 > 1 ) {
	noErr = false;
	break;
      }
    }
    for ( int ii = 0; ii < hmw->nuroprpar; ii++) {
      QString strdat;
      QString strFldat;
      if ( orig[selParNo[ii]] == -999.9 ||  orig[selParNo[ii]] == -32767.0 || controlinfo[selParNo[ii]].substr(6,1) == "1" || controlinfo[selParNo[ii]].substr(6,1) == "3" )
	strdat = "";
      else {
	if ( paramIsCode(selParNo[ii]) == 0 )
	  strdat = strdat.setNum(orig[selParNo[ii]],'f',0);
	else {
	  strdat = strdat.setNum(orig[selParNo[ii]],'f',1);
	}
      }
      TableItem* iorig = new TableItem(this, Q3TableItem::Never, strdat);
      iorig->isModelVal = false;
      iorig->isCorrectedByQC2 = false;
      if ( morig[selParNo[ii]] == -999.9 ||  morig[selParNo[ii]] == -32767.0 )
	strdat = "";
      else
	strdat = strdat.setNum(morig[selParNo[ii]],'f',1);
      TableItem* imorig = new TableItem(this, Q3TableItem::Never, strdat);
      imorig->isModelVal = false;
      imorig->isCorrectedByQC2 = false;
      int iflag = flag[selParNo[ii]];
      strdat = strdat.setNum(iflag);
      if ( iflag < 10 )
	strdat = "0000" + strdat;
      else if (iflag < 100 )
	strdat = "000" + strdat;
      else if ( iflag < 1000 )
	strdat = "00" + strdat;
      else if ( iflag < 10000 )
	strdat = "0" + strdat;
      if (strdat == "00000" ) strdat ="";
      TableItem* iflg = new TableItem(this, Q3TableItem::Never, strdat);
      iflg->isModelVal = false;
      iflg->isCorrectedByQC2 = false;
      strFldat = strdat;
      if ( corr[selParNo[ii]] == -999.9  ||  corr[selParNo[ii]] <= -32766.0 )
	strdat = "";
      else {
	if ( paramIsCode(selParNo[ii]) == 0 )
	  strdat = strdat.setNum(corr[selParNo[ii]],'f',0);
	else {
	  strdat = strdat.setNum(corr[selParNo[ii]],'f',1);
	}
      }
      TableItem* ikorr = new TableItem(this, Q3TableItem::OnTyping, strdat);
      if ( strFldat == "05000" )
	ikorr->isModelVal = true;
      else
	ikorr->isModelVal = false;
      if ( !controlinfo[selParNo[ii]].empty() && (controlinfo[selParNo[ii]].substr(7,1) > "0" ||
						  controlinfo[selParNo[ii]].substr(8,1) > "1" ||
						  controlinfo[selParNo[ii]].substr(9,1) > "1" ||
						  controlinfo[selParNo[ii]].substr(11,1) > "1" ||
						  controlinfo[selParNo[ii]].substr(12,1) > "6") ) {
	iorig->isCorrectedByQC2 = true;
	iflg->isCorrectedByQC2 = true;
	ikorr->isCorrectedByQC2 = true;
      }
      else {
	iorig->isCorrectedByQC2 = false;
	iflg->isCorrectedByQC2 = false;
	ikorr->isCorrectedByQC2 = false;
      }
      setItem(dt, noColPar*ii + 0, iorig);
      if ( ncp == 0 || ncp == 2 || ncp == 4 || ncp == 6 )
	hideColumn(noColPar*ii); 
      setItem(dt, noColPar*ii + 1, iflg);
      if ( ncp == 0 || ncp == 1 || ncp == 4 || ncp == 5  )
	hideColumn(noColPar*ii + 1);
      setItem(dt, noColPar*ii + 2, ikorr);
      //      setText(dt, noColPar*ii + 2, strdat);
      setItem(dt, noColPar*ii + 3, imorig);
      if ( !paramHasModel(selParNo[ii]) || ncp == 0 || ncp == 1 || ncp == 2 || ncp == 3 )
	hideColumn(noColPar*ii + 3);
      QString strClock = strTime.mid(11,2);
      if ( ((selParNo[ii] == 214 || selParNo[ii] == 216 || selParNo[ii] == 109 || selParNo[ii] == 110) && !(strClock == "06" || strClock == "18")) ||
	   (selParNo[ii] == 112  && strClock != "06") ) {
	iorig->setEnabled(false); 
	iflg->setEnabled(false); 
	imorig->setEnabled(false); 
      }
    }
    TableItem* istat = new TableItem(this, Q3TableItem::Never, strStnr);
    setItem(dt, noColPar*hmw->nuroprpar, istat);
    hideColumn(noColPar*hmw->nuroprpar);
    TableItem* itime = new TableItem(this, Q3TableItem::Never, strTime);
    setItem(dt, noColPar*hmw->nuroprpar+1, itime);
    hideColumn(noColPar*hmw->nuroprpar+1);
    TableItem* typId = new TableItem(this, Q3TableItem::Never, strTypeId);
    setItem(dt, noColPar*hmw->nuroprpar+2, typId);
    TableItem* staId = new TableItem(this, Q3TableItem::Never, strStnr);
    setItem(dt, noColPar*hmw->nuroprpar+3, staId);
      hideColumn( noColPar*hmw->nuroprpar+2 );
      hideColumn( noColPar*hmw->nuroprpar+2 );
    pistnr = istnr;
    if ( !strTime.endsWith("00") && !RR01inList )
      hideRow(dt);
    if ( noErr && (lity == erLo) )
      hideRow(dt);
  }
  hideRow(noSel);

  originalIndexes.reserve( noSel );
  for ( int i = 0; i < noSel; i++ )
    originalIndexes.push_back( i );
}

DataTable::~DataTable() {
}

bool DataTable::paramHasModel(int parNo) {
  for ( int i = 0; i < 8; i++ ) {
    if ( parNo == modPar[i] ) 
      return true;
  }
  return false;
}

int DataTable::paramIsCode(int parNo) {
  for ( int i = 0; i < 58; i++ ) {
    if ( parNo == codeParam[i] ) 
      return 0;
  }
  return 1;
}

void DataTable::swapRows( int row1, int row2, bool /*swapHeader*/ ) {

  swap( originalIndexes[row1], originalIndexes[row2] );  
  Q3Table::swapRows( row1, row2, TRUE );
}

void DataTable::sortColumn( int col, bool ascending, bool /*wholeRows*/ ) {
  Q3Table::sortColumn( col, ascending, TRUE );
}

kvalobs::kvData DataTable::getKvData() const
{
  return getKvData( currentRow(), currentColumn() );
}

kvalobs::kvData DataTable::getKvData( int row, int col ) const
{
  if ( row < 0 or row >= numRows() or col < 0 or col >= numCols() )
    return kvalobs::kvData();

  int dataListIndex = originalIndex( row );
  datl & d = getHqcMainWindow( this )->datalist[ dataListIndex ];

  HqcMainWindow * hmw = getHqcMainWindow( this );
  int index = (col-2)/hmw->nucoprpar;

  int                      pos = d.stnr;
  const miutil::miTime &   obt = d.otime;
  float                    org = d.orig[parNo[index]];
  int                      par = parNo[index];
  const miutil::miTime &   tbt = d.tbtime;     
  int                      typ = d.typeId[parNo[index]];     
  int                      sen = d.sensor[parNo[index]];    
  int                      lvl = d.level[parNo[index]];     
  float                    cor = d.corr[parNo[index]]; 
  kvControlInfo            cif = d.controlinfo[parNo[index]];
  const kvUseInfo &        uin = d.useinfo[parNo[index]];
  const miutil::miString & fai = d.cfailed[parNo[index]];
  const kvControlInfo & cin = cif;

  kvalobs::kvData ret(pos,obt,org,par,tbt,typ,sen,lvl,cor,cin,uin,fai);
  return ret;
}
  

int DataTable::originalIndex( int row ) const
{
  return originalIndexes[ row ];
}

void DataTable::selectStation( int station, const miutil::miTime & obstime )
{
  cerr << "SelectStation\n";
  const vector<datl> & data = getHqcMainWindow( this )->datalist;
  for ( int i = 0; i < originalIndexes.size(); ++i ) {
    const datl & d = data[ originalIndex( i ) ];
    if ( d.stnr == station and d.otime == obstime ) {
      cerr << "Was on row   " << currentRow() << endl
	   << "Going to row " << i << "(" << originalIndex( i ) << ")" << endl
	   << "(table has " << numRows() << " rows)" << endl;
      setCurrentCell( i, max( currentColumn(), 0 ) );
      selectRow( i );
      break;
    }
  }
}

void DataTable::toggleSort() {

  HqcMainWindow * hmw = getHqcMainWindow( this );

  if ( timeSort ) {
    sortColumn(hmw->nucoprpar*hmw->nuroprpar, TRUE, TRUE);
    timeSort = FALSE;
  }
  else {
    sortColumn(hmw->nucoprpar*hmw->nuroprpar + 1, TRUE, TRUE);
    timeSort = TRUE;
  }
}

void DataTable::focusTable(QString& cmn) {
  int comma = cmn.find(",",0);
  QString stnr = cmn.left(comma).stripWhiteSpace();
  QString time = cmn.right(cmn.length() - comma - 1).stripWhiteSpace();
  time.truncate(16);
  int row = 0;
  for ( int irow = 0; irow < numRows(); irow++) {
    Q3TableItem* tstat = item( irow, 0);
    Q3TableItem* ttime = item( irow, 0);
    QString qstat = verticalHeader()->label(irow).left(6).stripWhiteSpace();
    QString qsyno = "0" + verticalHeader()->label(irow).right(10).stripWhiteSpace();
    QString qtime;
    if ( qsyno == "0" )
      qtime = verticalHeader()->label(irow).right(22);
    else
      qtime = verticalHeader()->label(irow).right(27).left(16);
    if ( (qstat == stnr || qsyno == stnr ) && qtime == time ) {
      row = irow;
      break;
    }
  }
  ensureCellVisible(row, 0);
  selectRow(row);
}

QString TableItem::key() const {
  HqcMainWindow * hmw = getHqcMainWindow( table() );
  QString item;
  if ( col() < hmw->nucoprpar*hmw->nuroprpar + 1){
    item.sprintf("%08.1f",text().toDouble()+33000);
    }
    else {
      item = text();
    }
  return item;
}

void TableItem::paint( QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected )
{
    QColorGroup g( cg );
    if ( isModelVal )
      g.setColor( QColorGroup::Text, Qt::red );
    else
      g.setColor( QColorGroup::Text, Qt::black );
    if ( isCorrectedByQC2 )
      g.setColor( QColorGroup::Text, Qt::darkMagenta );

    Q3TableItem::paint( p, g, cr, selected );
}
