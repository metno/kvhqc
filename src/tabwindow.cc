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
/*! \file tabwindow.cc
 *  \brief Code for the MDITabWindow class.
 *  
 *  Displays the window where the tables are shown.
 *
*/

#include "tabwindow.h"
#include "datatable.h"
#include "BusyIndicator.h"

MDITabWindow::MDITabWindow( QWidget* parent, 
			    const char* name, 
			    int wflags, 
			    const miutil::miTime& stime, 
			    const miutil::miTime& etime,
			    miutil::miTime& remstime, 
			    miutil::miTime& remetime,
			    listType lity,
			    listType remLity,
			    mettType metty,
			    QString& wElement,
			    QStringList& selPar,
			    int noSelPar,
			    int* selParNo,
			    vector<datl>& datalist,
			    vector<modDatl>& modeldatalist,
			    list<kvStation>& slist,
			    int dateCol,
			    int ncp,
			    bool isShTy,
			    QString& userName)
  : QMainWindow( parent, name, wflags )
    , dtt( NULL ), erl( NULL ), erHead( NULL )
{
  readLimits();
  
  //  cerr << "Parent: " << parent->className() << endl;
  
  if ( lity == erLi || lity == erSa) {
    if (metty == tabHead ) {
      erHead = new ErrorHead(stime,etime,this,lity,userName);
      setFocusProxy( erHead );
      setCentralWidget( erHead );
    }
    else if ( metty == tabList ) {
      if ( erl ) 
	delete erl;
      erl = new ErrorList(selPar,
			  stime,
			  etime, 
			  numErr, 
			  noParam, 
			  this, 
			  lity, 
			  metty, 
			  selParNo,
			  datalist, 
			  modeldatalist,
			  slist, 
			  dateCol, 
			  ncp,
			  userName);
      cerr << "mditab: " << this << endl;
      
      HqcMainWindow * w = getHqcMainWindow( this );
      connect( w, SIGNAL( saveData() ), erl, SLOT( saveChanges() ) );
      //KTEST
      connect( w, SIGNAL( printErrorList() ), erl, SLOT( printErrorList() ) );
      
      connect( erl, SIGNAL( stationSelected( int, const miutil::miTime & ) ),
	       w, SIGNAL( errorListStationSelected(int, const miutil::miTime &) ) );
      
      setFocusProxy( erl );
      setCentralWidget( erl );
    }
  }
  else {
    readErrorsFromqaBase(numErr, noParam, stime, etime, remstime, remetime, lity, remLity, wElement);
    if ( dtt )
      delete dtt;
    dtt = new DataTable(selPar, 
			numErr,
			noSelPar,
			selParNo, 
			noParam, 
			this, 
			lity, 
			metty, 
			dateCol, 
			ncp,
			isShTy);
    
    HqcMainWindow * w = getHqcMainWindow( this );
    
    connect( dtt, SIGNAL( currentChanged( int, int) ),
	     SLOT( tableCellClicked( int, int ) ) );
    connect( dtt, SIGNAL( valueChanged( int, int ) ),
	     this,SLOT( updateKvBase( int, int ) ) );
    connect( dtt, SIGNAL( valueChanged( int, int ) ),
	     SLOT( tableValueChanged( int, int ) ) );
    connect( w, SIGNAL( errorListStationSelected(int, const miutil::miTime &) ),
	     dtt, SLOT( selectStation(int, const miutil::miTime &) ) );
    setFocusProxy( dtt );
    setCentralWidget( dtt );
  }
  //Remember stime and etime
}

void MDITabWindow::readErrorsFromqaBase(int& numerr, 
					int& noParam, 
					const miutil::miTime& stime, 
					const miutil::miTime& etime, 
					miutil::miTime& remstime, 
					miutil::miTime& remetime, 
					listType lity, 
					listType remLity, 
					QString wElement) {
  HqcMainWindow *mainWindow = (HqcMainWindow*)(parent()->parent()->parent());
  
  if (mainWindow-> timeFilterChanged || 
      mainWindow->kvBaseUpdated() ||
      remstime != stime || remetime != etime || lity != remLity ||
      !(mainWindow->stList 
	== mainWindow->remstList) ) {
    mainWindow->datalist.clear();
    mainWindow->timeFilterChanged = FALSE;
  }
  mainWindow->setKvBaseUpdated(FALSE);
  numErr = mainWindow->datalist.size();
  cerr << "Numerr før   = " << numerr << "  " << numErr <<endl;
  if ( wElement == "Alt" ) 
    noParam = NOPARAMALL;
  else if ( wElement == "Lufttrykk" ) 
    noParam = NOPARAMAIRPRESS;
  else if ( wElement == "Temperatur" ) 
    noParam = NOPARAMTEMP;
  else if ( wElement == "Nedbør" ) 
    noParam = NOPARAMPREC;
  else if ( wElement == "Visuell" ) 
    noParam = NOPARAMVISUAL;
  else if ( wElement == "Sjøgang" ) 
    noParam = NOPARAMWAVE;
  else if ( wElement == "Synop" ) 
    noParam = NOPARAMSYNOP;
  else if ( wElement == "Klimastatistikk" ) 
    noParam = NOPARAMKLSTAT;
  else if ( wElement == "Prioriterte parametere" ) 
    noParam = NOPARAMPRIORITY;
  else if ( wElement == "Vind" ) 
    noParam = NOPARAMWIND;
  else if ( wElement == "Pluviometerkontroll" ) 
    noParam = NOPARAMPLU;
  int numStat = mainWindow->stList.size();
  if ( numerr == 0 ) {
    mainWindow->readFromData(stime, etime, lity);
    mainWindow->readFromModelData(stime, etime);
    numErr = mainWindow->datalist.size();
  }
  cerr << "Numerr etter = " << numerr << "  " << numErr << endl;
  remstime = stime;
  remetime = etime;
  remLity = lity;
}

bool MDITabWindow::close( bool alsoDelete )
{
  if ( erl ) {
    bool close = erl->maybeSave();
    if ( not close )
      return false;
  }
  return QMainWindow::close( alsoDelete );
}

MDITabWindow::~MDITabWindow()
{
  //    delete mmovie;
}

//Generate message for showing observations in diana
void MDITabWindow::showObservations(int row) {
  
  //time
  QString dateTime = 
    dtt->verticalHeader()->label(row).right(21).left(16) + ":00";
  miutil::miTime time(dateTime);
  
  //hvilken stasjon
  QString stationNo = dtt->verticalHeader()->label(row).left(6);
  
  int stnr = stationNo.toInt();
  ((HqcMainWindow*)
   (parent()->parent()->parent()->parent()))->sendObservations(time,true);
  ((HqcMainWindow*)
   (parent()->parent()->parent()->parent()))->sendStation(stnr);
  
}

void MDITabWindow::showChangedValue(int row, int col, QString val) {
  
  QString stationNo = dtt->verticalHeader()->label(row).left(6);
  int stnr = stationNo.toInt();
  
  QString dateTime = 
    dtt->verticalHeader()->label(row).stripWhiteSpace().right(21).left(16);
  dateTime.append(":00");
  miutil::miTime time = miutil::miTime(dateTime.latin1());
  
  QTableItem* tflag      = dtt->item( row, col - 1);
  miutil::miString flag      = tflag->text().latin1();
  
  QString lbl = dtt->horizontalHeader()->label(col);
  int nlind = lbl.find('\n');
  lbl.truncate(nlind);
  miutil::miString param = lbl.latin1();
  
  miutil::miString value = val.latin1();

  getHqcMainWindow( this )->updateParams(stnr, time, param, value, flag);
}



void MDITabWindow::showSelectedParam(int row, int col) {
  QString lbl = dtt->horizontalHeader()->label(col);
  int nlind = lbl.find('\n');
  lbl.truncate(nlind);
  getHqcMainWindow( this )->sendSelectedParam(lbl.latin1());
}


void MDITabWindow::headerClicked(int row ) {
  showObservations(row);
}


void MDITabWindow::tableCellClicked(int row, 
				    int col, 
				    int button, 
				    const QPoint& mousePos) {
  HqcMainWindow * hqcm = getHqcMainWindow( this );
  if ( col%hqcm->nucoprpar == 2 ) {
    showSelectedParam(row, col);
  }
}

void MDITabWindow::tableCellClicked(int row, 
				    int col) {
  HqcMainWindow * hqcm = getHqcMainWindow( this );
  if ( col%hqcm->nucoprpar == 2 ) {
    showSelectedParam(row, col);
  }
}

void MDITabWindow::readLimits() {
  QString path = QString(getenv("HQCDIR"));
  if ( !path ) {
    cerr << "Intet environment" << endl;
    exit(1);
  }
  int par, dum;
  float low, high;
  QString limitsFile = path + "/slimits";
  QFile limits(limitsFile);
  if ( !limits.open(IO_ReadOnly) ) {
    cerr << "kan ikke åpne " << limitsFile << endl;
    exit(1);
  }
  QTextStream limitStream(&limits);
  while ( limitStream.atEnd() == 0 ) {
    limitStream >> par >> dum >> low >> high;
    lowMap[par] = low;
    highMap[par] = high;
  }
}

void MDITabWindow::updateKvBase(int row, int col) {
  
  HqcMainWindow * hqcm = getHqcMainWindow( this );
  
  QDoubleValidator inpVal(this);
  int index = (col-2)/hqcm->nucoprpar;
  QTableItem* tval = dtt->item( row, col);
  QString corVal   = tval->text();
  QString newFlagVal;//   = fval->text();
  int pos     = hqcm->datalist[row].stnr;
  const miutil::miTime & obt = hqcm->datalist[row].otime;
  int typ = hqcm->datalist[row].typeId[hqcm->selParNo[index]];
  double orig = hqcm->datalist[row].orig[hqcm->selParNo[index]];
  double corr = hqcm->datalist[row].corr[hqcm->selParNo[index]];
  QString oldCorVal;
  oldCorVal = oldCorVal.setNum(corr,'f',1);

  if ( pos > 99999 ) {
    QMessageBox::information( this,
			      "Utenlandsk stasjon",
			      "Utenlandske stasjoner kan ikke korrigeres",
			      QMessageBox::Ok,
			      QMessageBox::NoButton );
    tval->setText(oldCorVal);
    return;
  }

  if ( hqcm->reinserter != NULL ) {
    
    // Create a kvData object
    
    int dataListIndex = dtt->originalIndex( row );
    cerr << row << " - " << dataListIndex << endl;
    
    datl &d = hqcm->datalist[ dataListIndex ];
    
    int                      pos = d.stnr;
    const miutil::miTime &   obt = d.otime;
    float                    org = d.orig[hqcm->selParNo[index]];  
    int                      par = hqcm->selParNo[index];
    const miutil::miTime &   tbt = d.tbtime;     
    //    int                      typ = d.typeId;     
    int                      typ = d.typeId[hqcm->selParNo[index]];     
    int                      sen = d.sensor[hqcm->selParNo[index]];    
    int                      lvl = d.level[hqcm->selParNo[index]];      
    float                    cor = corVal.toFloat(); 
    kvControlInfo            cif = d.controlinfo[hqcm->selParNo[index]];
    const kvUseInfo &        uin = d.useinfo[hqcm->selParNo[index]];
    const miutil::miString & fai = d.cfailed[hqcm->selParNo[index]];
    if ( typ == -32767 ) typ = hqcm->findTypeId(typ, pos, par, obt);
    if ( cor == -32766 ) {
      cif.set(15,10);
      const int misfl = cif.flag(6);
      if ( misfl == 0 || misfl == 1 )
	cif.set(6,misfl + 2);
    }
    else {
      cif.set(15,7);
   }
    if ( oldCorVal == "-32767.0" ) { 
      cif.set(15,5);                      //Interpol
      int misfl;
      if ( cif.flag(6) == 0 ) 
	misfl = 1;
      else
	misfl  = cif.flag(6) - 2;
      cif.set(6,misfl);
     }
    const kvControlInfo & cin = cif;
    cerr << "Nytt flagg    = " << cif << endl;
    
    float uplim = highMap[par];
    float downlim = lowMap[par];
    int dum;
    if ( inpVal.validate(corVal,dum) == QValidator::Invalid  || corVal == "e" || corVal == "E" ) {
      QMessageBox::information( this,
				"Konverteringsfeil",
				"Verdien er ikke en tallverdi",
				QMessageBox::Ok,
				QMessageBox::NoButton );
      if ( oldCorVal == "-32767.0" )
	tval->setText("");
      else
	tval->setText(oldCorVal);
      return;
    }
    if ( (cor > uplim || cor < downlim) && cor != -32766 && par != 105) {
      QMessageBox::information( this,
				"Ulovlig verdi",
				"Verdien er utenfor fysikalske grenser",
				QMessageBox::Ok,
				QMessageBox::NoButton );
      if ( oldCorVal == "-32767.0" )
	tval->setText("");
      else
	tval->setText(oldCorVal);
      return;
    }
    if ( !legalTime(obt.hour(), par )) {
      QMessageBox::information( this,
				"Ulovlig tidspunkt",
				"Denne parameteren kan ikke lagres ved dette tidspunktet",
				QMessageBox::Ok,
				QMessageBox::NoButton );
      if ( oldCorVal == "-32767.0" )
	tval->setText("");
      else
	tval->setText(oldCorVal);
      return;
    }
    if ( !legalValue( cor, par )) {
      QMessageBox::information( this,
				"Ulovlig verdi",
				"Lovlige verdier er -5 og -6",
				QMessageBox::Ok,
				QMessageBox::NoButton );
      if ( oldCorVal == "-32767.0" )
	tval->setText("");
      else
	tval->setText(oldCorVal);
      return;
    }
  
    QString newCorVal;
    newCorVal = newCorVal.setNum(cor,'f',1);
    QString changeVal;
    if ( cor == -32766 )
      changeVal = "Vil du forkaste " + oldCorVal + " ?";
    else
      changeVal = "Vil du  endre " + oldCorVal + " til " + newCorVal + " ?";
    
    int corrMb = 
      QMessageBox::information( this,
				"Korrigering",
				changeVal,
				"Ja",
				"Nei",
				"" );
    if ( corrMb == 1 ) {
      tval->setText(oldCorVal);
      return;
    }
    if ( cor == -32766 )
      tval->setText("");
    int newFlag = hqcm->getShowFlag(cin);
    newFlagVal = newFlagVal.setNum(newFlag);
    TableItem* fval = new TableItem( dtt, QTableItem::Never, newFlagVal);
    fval->setText(newFlagVal);
    dtt->setItem(row, col - 1, fval);
    kvalobs::kvData kd(pos,obt,org,par,tbt,typ,sen,lvl,cor,cin,uin,fai);
    cerr << decodeutility::kvdataformatter::createString( kd ) << endl;
    
    CKvalObs::CDataSource::Result_var result;	 
    {	 
      BusyIndicator busyIndicator;	 
      result = hqcm->reinserter->insert(kd);	 
    }	 
    if ( result->res != CKvalObs::CDataSource::OK ) {	 
      QMessageBox::critical( this, 	 
			     "Kan ikke lagre data",	 
			     QString( "Kan ikke lagre data!\n"	 
				      "Meldingen fra Kvalobs var:\n" ) +	 
			     result->message,	 
			     QMessageBox::Ok,	 
			     QMessageBox::NoButton );	 
      tval->setText(oldCorVal);	 
      return;	 
    }    hqcm->setKvBaseUpdated(TRUE);
  }
  else {
    tval->setText(oldCorVal);
    cerr << "Du er ikke autorisert til å skrive i databasen!" << endl;
  }
}

void MDITabWindow::tableValueChanged(int row, int col) {
  QTableItem* tval = dtt->item( row, col);
  QString val      = tval->text();
  if ( col > 0 && col%4 != 2 ) {
    QTableItem* tflag = dtt->item( row, col - 1);
    QString flag = tflag->text();
    int oldFlag = flag.toInt();
    int newFlag = oldFlag%10 == 0 ? oldFlag + 1 : oldFlag;
    flag = flag.setNum(newFlag);
    if ( newFlag < 10 )
      flag = "0000" + flag;
    else if ( newFlag < 100 )
      flag = "000" + flag;
    else if ( newFlag < 1000 )
      flag = "00" + flag;
    else if ( newFlag < 10000 )
      flag = "0" + flag;
    TableItem* tnflag = new TableItem(dtt, QTableItem::Never, flag);
    dtt->setItem( row, col - 1, tnflag);
    double a = val.toDouble();
  }
  showChangedValue(row, col, val);
}

bool MDITabWindow::legalTime(int hour, int par) {
  bool lT = true;
  if ( (par == 214 || par == 216 || par == 224 || par == 109) && 
       !(hour == 6 || hour == 18) || par == 110 && hour != 6 ) lT = false;
    return lT;
}

bool MDITabWindow::legalValue(double val, int par) {
  bool lT = true;
  if ( par == 105 && ( val != -5.0 && val != -6.0 && val != -32766.0 ) )
    lT = false;
  return lT;
}
