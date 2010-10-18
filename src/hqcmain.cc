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
/*! \file hqcmain.cc
 *  \brief Code for the HqcMainWindow class.
 *  
 *  Displays the main window, the essence of the app.
 *
*/

#include <algorithm>
#include "helptree.h"
#include "hqcmain.h"
#include "StationInformation.h"
#include "GetData.h"
#include "GetTextData.h"
#include <iomanip>
#include <qpixmap.h>
#include <QWindowsXPStyle>
#include <qwindowsstyle.h>
#include <qcdestyle.h>
#include <qcommonstyle.h>
#include <qvalidator.h>
#include <qmetaobject.h>
#include <qlistview.h>
//Added by qt3to4:
#include <QFrame>
#include <Q3PopupMenu>
#include <qTimeseries/TSPlot.h>
#include <glText/glTextX.h>
#include <kvalobs/kvData.h>
#include "identifyUser.h"
#include "BusyIndicator.h"
#include "RRDialog.h"
#include "weatherdialog.h"
#include <deque>
#include <stdexcept>
#include <complex>
#include <QTime>
#include <QRegExp>

#include <glText/glTextQtTexture.h>

using namespace std;

int noSelPar;
int modelParam[] = {61,81,109,110,177,178,211,262};
miutil::miTime remstime;
miutil::miTime remetime;
miutil::miTime remdlstime;
miutil::miTime remdletime;
listType remLity;


HqcMainWindow * getHqcMainWindow( const QObject * o )
{
  QObject * iterator = 0;
  if ( o )
    iterator = o->parent();

  while ( iterator != 0 ) {
    if ( iterator->isA( "HqcMainWindow" ) )
      return dynamic_cast<HqcMainWindow *>( iterator );
    iterator = iterator->parent();
  }
  return 0;
}

HqcMainWindow * getHqcMainWindow( QObject * o )
{
  if ( o->isA( "HqcMainWindow" ) )
    return dynamic_cast<HqcMainWindow *>( o );
  return getHqcMainWindow( const_cast<const QObject *>( o ) );
}


/**
 * Main Window Constructor.
 */	
  HqcMainWindow::HqcMainWindow()
  : Q3MainWindow( 0, "HQC")
  , reinserter( NULL )
{
  readFromStation();
  //  setAttribute(Qt::WA_DeleteOnClose);
  // --- CHECK USER IDENTITY ----------------------------------------

  reinserter = Authentication::identifyUser(  KvApp::kvApp,
					     "ldap-oslo.met.no", userName);
  if ( reinserter == NULL ) {
    int mb = QMessageBox::information(this, 
				  "Autentisering", 
				  "Du er ikke registrert som operatør!\n"
				  "Du kan se dataliste, feillog og feilliste,\n"
				  "men ikke gjøre endringer i Kvalobsdatabasen!",
				  "Fortsett",
				  "Avslutt",
				  "");
    if ( mb == 1 )
      throw runtime_error( "Not authenticated" );
  }
  else {
    cout << "Hei  " << userName.toStdString() << ", du er registrert som godkjent operatør" << endl;
  }
  //-----------------------------------------------------------------  

  firstObs = true;
  sLevel  = 0;
  sSensor = 0;
  QStringList dName, dNum;
  remstime.setTime(miutil::miString("2000-01-01 00:00:00"));
  remetime.setTime(miutil::miString("2000-01-01 00:00:00"));
  remdlstime.setTime(miutil::miString("2000-01-01 00:00:00"));
  remdletime.setTime(miutil::miString("2000-01-01 00:00:00"));
  remLity = dumLi; 
  dianaconnected=  false;  // connection to diana
  tsVisible = false;

  // ---- MAIN MENU ---------------------------------------------
  //  qApp->setStyle(new QSGIStyle);

  listExist = FALSE;
  menuBar()->setFont(QFont("system", 12));
  
  file = new Q3PopupMenu( this );
  menuBar()->insertItem( "&Fil", file );
  file->insertSeparator();

  fileSaveMenuItem = file->insertItem( "Lagre", this, SIGNAL( saveData() ), Qt::CTRL+Qt::Key_S );
  file->setItemEnabled( fileSaveMenuItem, false );

  filePrintMenuItem = file->insertItem( "Skriv ut", this, SIGNAL( printErrorList() ), Qt::CTRL+Qt::Key_P );
  file->setItemEnabled( filePrintMenuItem, false );

  file->insertItem( "&Lukk",    this, SLOT(closeWindow()), Qt::CTRL+Qt::Key_W );
  file->insertItem( "&Avslutt", qApp, SLOT( closeAllWindows() ), Qt::CTRL+Qt::Key_Q );
  
  choice = new Q3PopupMenu( this );
  choice->setCheckable(TRUE);
  menuBar()->insertItem( "&Valg", choice );
  choice->insertSeparator();
  flID = choice->insertItem( "Vis flagg",                 this, SLOT(showFlags()));
  orID = choice->insertItem( "Vis original",              this, SLOT(showOrigs()));
  moID = choice->insertItem( "Vis modeldata",             this, SLOT(showMod()));
  stID = choice->insertItem( "Vis stasjonsnavn",          this, SLOT(showStat()));
  poID = choice->insertItem( "Vis lengde, bredde, høyde", this, SLOT(showPos()));

  isShFl = TRUE;
  isShOr = TRUE;
  isShMo = TRUE;
  isShSt = TRUE;

  choice->setItemChecked(flID, isShFl);
  choice->setItemChecked(orID, isShOr);
  choice->setItemChecked(moID, isShMo);
  choice->setItemChecked(stID, isShSt);
  
  showmenu = new Q3PopupMenu( this );
  menuBar()->insertItem( "&Listetype", showmenu);
  showmenu->insertItem( "Data&liste og Feilliste    ", this, SLOT(allListMenu()),Qt::ALT+Qt::Key_L );
  showmenu->insertItem( "&Feilliste    ", this, SLOT(errListMenu()),Qt::ALT+Qt::Key_F );
  showmenu->insertItem( "F&eillog    ",   this, SLOT(errLogMenu()),Qt::ALT+Qt::Key_E );
  showmenu->insertItem( "&Dataliste    ", this, SLOT(dataListMenu()),Qt::ALT+Qt::Key_D );
  showmenu->insertItem( "&Feilliste salen", this, SLOT(errLisaMenu()),Qt::ALT+Qt::Key_S );
  showmenu->insertSeparator();
  showmenu->insertItem( "&Nedbør", this, SLOT( showWatchRR() ), Qt::CTRL+Qt::Key_R );
  showmenu->insertItem( "&Vær", this, SLOT( showWeather() ), Qt::CTRL+Qt::Key_V );
  showmenu->insertSeparator();
  showmenu->insertItem( "&Tidsserie    ", this, SLOT(timeseriesMenu()),Qt::ALT+Qt::Key_T );
  showmenu->insertSeparator();
  showmenu->insertItem( "Te&xtData    ", this, SLOT(textDataMenu()),Qt::ALT+Qt::Key_X );
  showmenu->insertItem( "Re&jected    ", this, SLOT(rejectedMenu()),Qt::ALT+Qt::Key_J );
  
  weathermenu = new Q3PopupMenu( this );
  menuBar()->insertItem( "&Værelement", weathermenu);
  wElement = "";
  klID = weathermenu->insertItem( "&For daglig rutine",       this, SLOT(climateStatistics()) );
  piID = weathermenu->insertItem( "&Prioriterte parametere",  this, SLOT(priority()) );
  taID = weathermenu->insertItem( "&Temperatur og fuktighet", this, SLOT(temperature()) );
  prID = weathermenu->insertItem( "&Nedbør og snøforhold",    this, SLOT(precipitation()) );
  apID = weathermenu->insertItem( "&Lufttrykk og vind",       this, SLOT(airPress()) );
  clID = weathermenu->insertItem( "&Visuelle parametere",     this, SLOT(visuals()) );
  seID = weathermenu->insertItem( "&Maritime parametere",     this, SLOT(sea()) );
  syID = weathermenu->insertItem( "&Synop",                   this, SLOT(synop()) );
  wiID = weathermenu->insertItem( "&Vind",                    this, SLOT(wind()) );
  plID = weathermenu->insertItem( "&Pluviometerparametere",   this, SLOT(plu()) );
  alID = weathermenu->insertItem( "&Alt",                     this, SLOT(all()) );

  clockmenu = new Q3PopupMenu( this );
  menuBar()->insertItem( "&Tidspunkter", this, SLOT(clk()));
  menuBar()->insertItem( "&Dianavisning", this, SLOT(dsh()));
  menuBar()->insertItem( "&Kro", this, SLOT(startKro()));
  Q3PopupMenu * help = new Q3PopupMenu( this );
  menuBar()->insertItem( "&Hjelp", help );
  help->insertItem( "&Brukerveiledning", this, SLOT(helpUse()), Qt::Key_F1);
  help->insertItem( "&Flagg", this, SLOT(helpFlag()), Qt::Key_F2);
  help->insertItem( "&Parametere", this, SLOT(helpParam()), Qt::Key_F3);
  help->insertSeparator();
  help->insertItem( "&Om Hqc", this, SLOT(about()));
  help->insertSeparator();
  help->insertItem( "Om &Qt", this, SLOT(aboutQt()));
  
  // --- MAIN WINDOW -----------------------------------------
  ws = new QWorkspace(this);
  ws->setScrollBarsEnabled( TRUE );
  ws->setBackgroundColor( Qt::white );
  setCentralWidget( ws );

  connect( ws, SIGNAL( windowActivated(QWidget*) ),
	   this, SLOT( updateSaveFunction(QWidget*) ) );

  
  // --- TOOL BAR --------------------------------------------
  QPixmap icon_listdlg("/usr/local/etc/kvhqc/table.png");
  QPixmap icon_ts("/usr/local/etc/kvhqc/kmplot.png");
  Q3ToolBar * hqcTools = new Q3ToolBar( this, "hqc" );
  hqcTools->setLabel( "Hqcfunksjoner" );
  QToolButton* listButton;
  QToolButton* tsButton;
  listButton = new QToolButton( icon_listdlg, 
				tr("Dataliste"), 
				"", 
				this, 
				SLOT(dataListMenu()), 
				hqcTools );
  tsButton = new QToolButton( icon_ts, 
			      tr("Tidsserie"), 
			      "", 
			      this, 
			      SLOT(timeseriesMenu()), 
			      hqcTools );
  
  // --- STATUS BAR -------------------------------------------
  
  //  if(usesocket){
  miutil::miString name = "hqc";   
  miutil::miString command = "/usr/local/bin/coserver4";  
  pluginB = new ClientButton(name.cStr(),
			     command.cStr(),
			     statusBar());
  pluginB->useLabel(true);
  pluginB->connectToServer();
  
  connect(pluginB, SIGNAL(receivedMessage(miMessage&)),
	  SLOT(processLetter(miMessage&)));
  connect(pluginB, SIGNAL(addressListChanged()),
	  SLOT(processConnect()));
  connect(pluginB, SIGNAL(connectionClosed()),
	  SLOT(cleanConnection()));
  statusBar()->addWidget(pluginB,0,true);
  
  //    }
  statusBar()->message( "Ready", 2000 );
  
  // --- READ STATION INFO ----------------------------------------

  cerr.setf(ios::fixed);
  int statCheck = 0;
  readFromStationFile(statCheck);
  readFromObsPgm();
  int prostnr = -1;
  
  TypeList tpList;
  for(CIObsPgmList obit=obsPgmList.begin();obit!=obsPgmList.end(); obit++){
    int ostnr = obit->stationID();
    if ( ostnr != prostnr ) {
      if ( prostnr != -1 )
	otpList.push_back(tpList);
      tpList.clear();
      tpList.push_back(ostnr);
    }
    int otpid = obit->typeID();
    TypeList::iterator tpind = std::find(tpList.begin(), tpList.end(), otpid);
    if ( tpind ==  tpList.end() ) {
      tpList.push_back(otpid);
      TypeList::iterator it = tpList.begin();
    }
    prostnr = ostnr;
  } 
  otpList.push_back(tpList);
  
  int noStat = 0;
  for ( QStringList::Iterator sit = listStatName.begin(); 
	sit != listStatName.end(); 
	++sit ) {
    noStat++;
  }

  readFromParam();

  // --- DEFINE DIALOGS --------------------------------------------
  lstdlg = new ListDialog(this);
  lstdlg->setIcon( QPixmap("/usr/local/etc/kvhqc/hqc.png") );
  clkdlg = new ClockDialog(this);
  clkdlg->setIcon( QPixmap("/usr/local/etc/kvhqc/hqc.png") );
  pardlg = new ParameterDialog(this);
  pardlg->setIcon( QPixmap("/usr/local/etc/kvhqc/hqc.png") );
  dshdlg = new DianaShowDialog(this);
  dshdlg->setIcon( QPixmap("/usr/local/etc/kvhqc/hqc.png") );
  txtdlg = new TextDataDialog(stnrList, this);
  txtdlg->setIcon( QPixmap("/usr/local/etc/kvhqc/hqc.png") );
  rejdlg = new RejectDialog(this);
  rejdlg->setIcon( QPixmap("/usr/local/etc/kvhqc/hqc.png") );
  // --- READ PARAMETER INFO ---------------------------------------
  
 
  // --- START -----------------------------------------------------
  pardlg->hide();
  rejdlg->hide();
  dianaShowOK();
  lstdlg->hide();
  connect( lstdlg, SIGNAL(selectStation()), SLOT(stationOK()));
  
  connect( lstdlg, SIGNAL(ListApply()), SLOT(ListOK()));
  connect( lstdlg, SIGNAL(ListHide()), SLOT(listMenu()));
  
  clkdlg->hide();
  timeFilterChanged = FALSE;
  connect( clkdlg, SIGNAL(ClockApply()), SLOT(ClkOK()));
  connect( clkdlg, SIGNAL(ClockHide()), SLOT(clockMenu()));
  
  dshdlg->hide();
  connect( dshdlg, SIGNAL(dianaShowApply()), SLOT(dianaShowOK()));
  connect( dshdlg, SIGNAL(dianaShowHide()), SLOT(dianaShowMenu()));
  
  connect( pardlg, SIGNAL(paramApply()), SLOT(paramOK()));
  connect( pardlg, SIGNAL(paramHide()), SLOT(paramMenu()));

  connect( txtdlg, SIGNAL(textDataApply()), SLOT(textDataOK()));
  connect( txtdlg, SIGNAL(textDataHide()), SLOT(textDataMenu()));

  connect( rejdlg, SIGNAL(rejectApply()), SLOT(rejectedOK()));
  connect( rejdlg, SIGNAL(rejectHide()), SLOT(rejectedMenu()));

  tsdlg = new TimeseriesDialog();
  tsdlg->hide();

  connect( tsdlg, SIGNAL(TimeseriesApply()), SLOT(TimeseriesOK()));
  connect( tsdlg, SIGNAL(TimeseriesHide()), SLOT(timeseriesMenu()));
  connect( this,SIGNAL(newStationList(std::vector<QString>&)), 
	   tsdlg,SLOT(newStationList(std::vector<QString>&)));
  connect( this,SIGNAL(newParameterList(const QStringList&)), 
	   tsdlg,SLOT(newParameterList(const QStringList&)));

  connect( lstdlg, SIGNAL(fromTimeChanged(const miutil::miTime& )),
	   tsdlg,  SLOT(setFromTimeSlot(const miutil::miTime&)));

  connect( lstdlg, SIGNAL(toTimeChanged(const miutil::miTime& )),
	   tsdlg,  SLOT(setToTimeSlot(const miutil::miTime&)));

  // make the timeseries-plot-dialog
  tspdialog = new TSPlotDialog(this);
  // init fonts for timeseries
  glText* gltext= new glTextQtTexture;
  gltext->testDefineFonts();
}

void HqcMainWindow::setKvBaseUpdated(bool isUpdated) {
  kvBaseIsUpdated = isUpdated;
}

void HqcMainWindow::showFlags() {
  isShFl = !isShFl;
  choice->setItemChecked(flID, isShFl);
}

void HqcMainWindow::showOrigs() {
  isShOr = !isShOr;
  choice->setItemChecked(orID, isShOr);
}

void HqcMainWindow::showMod() {
  isShMo = !isShMo;
  choice->setItemChecked(moID, isShMo);
}

void HqcMainWindow::showStat() {
  isShSt = !isShSt;
  choice->setItemChecked(stID, isShSt);
}

void HqcMainWindow::showPos() {
  isShPo = !isShPo;
  choice->setItemChecked(poID, isShPo);
}

void HqcMainWindow::showTyp() {
  isShTy = !isShTy;
  choice->setItemChecked(tyID, isShTy);
}

void HqcMainWindow::insertParametersInListBox(int maxOrder, int* porder) {
  pardlg->plb->clear();
  for ( int jj = 0; jj < maxOrder; jj++ ) {
    const char* sp =  parMap[*(porder+jj)].latin1();
    pardlg->plb->insertItem(QString(sp));
  }
}

void HqcMainWindow::airPress() {
  wElement = "Lufttrykk";
  lity = daLi;
  firstObs = true;
  weathermenu->setItemChecked(apID, TRUE);
  weathermenu->setItemChecked(taID, FALSE);
  weathermenu->setItemChecked(wiID, FALSE);
  weathermenu->setItemChecked(prID, FALSE);
  weathermenu->setItemChecked(clID, FALSE);
  weathermenu->setItemChecked(seID, FALSE);
  weathermenu->setItemChecked(syID, FALSE);
  weathermenu->setItemChecked(klID, FALSE);
  weathermenu->setItemChecked(piID, FALSE);
  weathermenu->setItemChecked(alID, FALSE);
  weathermenu->setItemChecked(plID, FALSE);
  insertParametersInListBox(NOPARAMAIRPRESS, airPressOrder);
  pardlg->showAll();
  //  sendObservations(remstime,false);
}

void HqcMainWindow::temperature() {
  wElement = "Temperatur";
  lity = daLi;
  firstObs = true;
  weathermenu->setItemChecked(apID, FALSE);
  weathermenu->setItemChecked(taID, TRUE);
  weathermenu->setItemChecked(wiID, FALSE);
  weathermenu->setItemChecked(prID, FALSE);
  weathermenu->setItemChecked(clID, FALSE);
  weathermenu->setItemChecked(seID, FALSE);
  weathermenu->setItemChecked(syID, FALSE);
  weathermenu->setItemChecked(klID, FALSE);
  weathermenu->setItemChecked(piID, FALSE);
  weathermenu->setItemChecked(alID, FALSE);
  weathermenu->setItemChecked(plID, FALSE);
  insertParametersInListBox(NOPARAMTEMP, tempOrder);
  pardlg->showAll();
  //  sendObservations(remstime,false);
}

void HqcMainWindow::precipitation() {
  wElement = "Nedbør";
  lity = daLi;
  firstObs = true;
  weathermenu->setItemChecked(apID, FALSE);
  weathermenu->setItemChecked(taID, FALSE);
  weathermenu->setItemChecked(wiID, FALSE);
  weathermenu->setItemChecked(prID, TRUE);
  weathermenu->setItemChecked(clID, FALSE);
  weathermenu->setItemChecked(seID, FALSE);
  weathermenu->setItemChecked(syID, FALSE);
  weathermenu->setItemChecked(klID, FALSE);
  weathermenu->setItemChecked(piID, FALSE);
  weathermenu->setItemChecked(plID, FALSE);
  weathermenu->setItemChecked(alID, FALSE);
  //  sendObservations(remstime,false);
  insertParametersInListBox(NOPARAMPREC, precOrder);
  pardlg->showAll();
}

void HqcMainWindow::visuals() {
  wElement = "Visuell";
  lity = daLi;
  firstObs = true;
  weathermenu->setItemChecked(apID, FALSE);
  weathermenu->setItemChecked(taID, FALSE);
  weathermenu->setItemChecked(wiID, FALSE);
  weathermenu->setItemChecked(prID, FALSE);
  weathermenu->setItemChecked(clID, TRUE);
  weathermenu->setItemChecked(seID, FALSE);
  weathermenu->setItemChecked(syID, FALSE);
  weathermenu->setItemChecked(klID, FALSE);
  weathermenu->setItemChecked(piID, FALSE);
  weathermenu->setItemChecked(alID, FALSE);
  weathermenu->setItemChecked(plID, FALSE);
  //  sendObservations(remstime,false);
  insertParametersInListBox(NOPARAMVISUAL, visualOrder);
  pardlg->showAll();
}


void HqcMainWindow::sea() {
  wElement = "Sjøgang";
  lity = daLi;
  weathermenu->setItemChecked(apID, FALSE);
  weathermenu->setItemChecked(taID, FALSE);
  weathermenu->setItemChecked(wiID, FALSE);
  weathermenu->setItemChecked(prID, FALSE);
  weathermenu->setItemChecked(clID, FALSE);
  weathermenu->setItemChecked(seID, TRUE);
  weathermenu->setItemChecked(syID, FALSE);
  weathermenu->setItemChecked(klID, FALSE);
  weathermenu->setItemChecked(piID, FALSE);
  weathermenu->setItemChecked(alID, FALSE);
   weathermenu->setItemChecked(plID, FALSE);
   //  sendObservations(remstime,false);
  insertParametersInListBox(NOPARAMWAVE, waveOrder);
  pardlg->showAll();
}

void HqcMainWindow::synop() {
  wElement = "Synop";
  lity = daLi;
  firstObs = true;
  weathermenu->setItemChecked(apID, FALSE);
  weathermenu->setItemChecked(taID, FALSE);
  weathermenu->setItemChecked(wiID, FALSE);
  weathermenu->setItemChecked(prID, FALSE);
  weathermenu->setItemChecked(clID, FALSE);
  weathermenu->setItemChecked(seID, FALSE);
  weathermenu->setItemChecked(syID, TRUE);
  weathermenu->setItemChecked(klID, FALSE);
  weathermenu->setItemChecked(piID, FALSE);
  weathermenu->setItemChecked(alID, FALSE);
  weathermenu->setItemChecked(plID, FALSE);
  //  sendObservations(remstime,false);
  insertParametersInListBox(NOPARAMSYNOP, synopOrder);
  pardlg->showAll();
}

void HqcMainWindow::climateStatistics() {
  wElement = "Klimastatistikk";
  lity = daLi;
  firstObs = true;
  weathermenu->setItemChecked(apID, FALSE);
  weathermenu->setItemChecked(taID, FALSE);
  weathermenu->setItemChecked(wiID, FALSE);
  weathermenu->setItemChecked(prID, FALSE);
  weathermenu->setItemChecked(clID, FALSE);
  weathermenu->setItemChecked(seID, FALSE);
  weathermenu->setItemChecked(syID, FALSE);
  weathermenu->setItemChecked(klID, TRUE);
  weathermenu->setItemChecked(piID, FALSE);
  weathermenu->setItemChecked(alID, FALSE);
  weathermenu->setItemChecked(plID, FALSE);
  //  sendObservations(remstime,false);
  insertParametersInListBox(NOPARAMKLSTAT, klstatOrder);
  pardlg->showAll();
}

void HqcMainWindow::priority() {
  wElement = "Prioriterte parametere";
  lity = daLi;
  firstObs = true;
  weathermenu->setItemChecked(apID, FALSE);
  weathermenu->setItemChecked(taID, FALSE);
  weathermenu->setItemChecked(wiID, FALSE);
  weathermenu->setItemChecked(prID, FALSE);
  weathermenu->setItemChecked(clID, FALSE);
  weathermenu->setItemChecked(seID, FALSE);
  weathermenu->setItemChecked(syID, FALSE);
  weathermenu->setItemChecked(klID, FALSE);
  weathermenu->setItemChecked(piID, TRUE);
  weathermenu->setItemChecked(alID, FALSE);
  weathermenu->setItemChecked(plID, FALSE);
  //  sendObservations(remstime,false);
  insertParametersInListBox(NOPARAMPRIORITY, priorityOrder);
  pardlg->showAll();
}

void HqcMainWindow::wind() {
  wElement = "Vind";
  lity = daLi;
  firstObs = true;
  weathermenu->setItemChecked(apID, FALSE);
  weathermenu->setItemChecked(taID, FALSE);
  weathermenu->setItemChecked(wiID, TRUE);
  weathermenu->setItemChecked(prID, FALSE);
  weathermenu->setItemChecked(clID, FALSE);
  weathermenu->setItemChecked(seID, FALSE);
  weathermenu->setItemChecked(syID, FALSE);
  weathermenu->setItemChecked(klID, FALSE);
  weathermenu->setItemChecked(piID, FALSE);
  weathermenu->setItemChecked(alID, FALSE);
  weathermenu->setItemChecked(plID, FALSE);
  //  sendObservations(remstime,false);
  insertParametersInListBox(NOPARAMWIND, windOrder);
  pardlg->showAll();
}

void HqcMainWindow::plu() {
  wElement = "Pluviometerkontroll";
  lity = daLi;
  firstObs = true;
  weathermenu->setItemChecked(apID, FALSE);
  weathermenu->setItemChecked(taID, FALSE);
  weathermenu->setItemChecked(wiID, FALSE);
  weathermenu->setItemChecked(prID, FALSE);
  weathermenu->setItemChecked(clID, FALSE);
  weathermenu->setItemChecked(seID, FALSE);
  weathermenu->setItemChecked(syID, FALSE);
  weathermenu->setItemChecked(klID, FALSE);
  weathermenu->setItemChecked(piID, FALSE);
  weathermenu->setItemChecked(alID, FALSE);
  weathermenu->setItemChecked(plID, TRUE);
  //  sendObservations(remstime,false);
  insertParametersInListBox(NOPARAMPLU, pluOrder);
  pardlg->showAll();
}

void HqcMainWindow::all() {
  wElement = "Alt";
  lity = daLi;
  firstObs = true;
  weathermenu->setItemChecked(apID, FALSE);
  weathermenu->setItemChecked(taID, FALSE);
  weathermenu->setItemChecked(wiID, FALSE);
  weathermenu->setItemChecked(prID, FALSE);
  weathermenu->setItemChecked(clID, FALSE);
  weathermenu->setItemChecked(seID, FALSE);
  weathermenu->setItemChecked(syID, FALSE);
  weathermenu->setItemChecked(klID, FALSE);
  weathermenu->setItemChecked(piID, FALSE);
  weathermenu->setItemChecked(plID, FALSE);
  weathermenu->setItemChecked(alID, TRUE);
  //  sendObservations(remstime,false);
  insertParametersInListBox(NOPARAMALL, order);
  pardlg->showAll();
}

void HqcMainWindow::paramOK() {
  if ( listExist )
    ListOK();
}

void HqcMainWindow::ClkOK() {
  timeFilterChanged = TRUE;
}

void HqcMainWindow::dianaShowOK() {
  dnMap["TTT"] = "TA";
  mdMap["TTT"] = false;
  diMap["TTT"] = false;
  if ( dshdlg->tameType->isChecked() )
    dnMap["TTT"] = "TAM";
  if ( dshdlg->tamoType->isChecked() ) {
    dnMap["TTT"] = "TA";
    mdMap["TTT"] = true;
  }
  if ( dshdlg->tadiType->isChecked() ) {
    dnMap["TTT"] = "TA";
    diMap["TTT"] = true;
  }

  dnMap["TdTdTd"] = "TD";
  mdMap["TdTdTd"] = false;
  if ( dshdlg->uuType->isChecked() )
    dnMap["TdTdTd"] = "UU";
  if ( dshdlg->uumoType->isChecked() ) {
    dnMap["TdTdTd"] = "UU";
    mdMap["TdTdTd"] = true;
  }
  if ( dshdlg->uumeType->isChecked() )
    dnMap["TdTdTd"] = "UM";
  if ( dshdlg->uumiType->isChecked() )
    dnMap["TdTdTd"] = "UN";
  if ( dshdlg->uumaType->isChecked() )
    dnMap["TdTdTd"] = "UX";
  
  dnMap["PPPP"] = "PR";
  mdMap["PPPP"] = false;
  diMap["PPPP"] = false;
  if ( dshdlg->poType->isChecked() )
    dnMap["PPPP"] = "PO";
  if ( dshdlg->prmoType->isChecked() ) {
    dnMap["PPPP"] = "PR";
    mdMap["PPPP"] = true;
  }
  if ( dshdlg->podiType->isChecked() ) {
    dnMap["PPPP"] = "PR";
    diMap["PPPP"] = true;
  }
  if ( dshdlg->pomeType->isChecked() )
    dnMap["PPPP"] = "POM";
  if ( dshdlg->pomiType->isChecked() )
    dnMap["PPPP"] = "PON";
  if ( dshdlg->pomaType->isChecked() )
    dnMap["PPPP"] = "POX";
  if ( dshdlg->phType->isChecked() )
    dnMap["PPPP"] = "PH";
  
  dnMap["ppp"] = "PP";
  mdMap["ppp"] = false;
  if ( dshdlg->ppmoType->isChecked() ) {
    dnMap["ppp"] = "PP";
    mdMap["ppp"] = true;
  }

  dnMap["RRR"] = "RR_12";
  mdMap["RRR"] = false;
  if ( dshdlg->rrmoType->isChecked() ) {
    dnMap["RRR"] = "RR_12";
    mdMap["RRR"] = true;
  }
  if ( dshdlg->rr1Type->isChecked() )
    dnMap["RRR"] = "RR_1";
  if ( dshdlg->rr6Type->isChecked() )
    dnMap["RRR"] = "RR_6";
  if ( dshdlg->rr24Type->isChecked() )
    dnMap["RRR"] = "RR_24";
  if ( dshdlg->rr24moType->isChecked() ) {
    dnMap["RRR"] = "RR_24";
    mdMap["RRR"] = true;
  }
  if ( dshdlg->rrprType->isChecked() ) {
    dnMap["RRR"] = "RR_12";
    prMap["RRR"] = true;
  }

  dnMap["TxTn"] = "TAN_12";
  if ( dshdlg->tx12Type->isChecked() ) {
    dnMap["TxTn"] = "TAX_12";
  }
  if ( dshdlg->tnType->isChecked() )
    dnMap["TxTn"] = "TAN";
  if ( dshdlg->txType->isChecked() )
    dnMap["TxTn"] = "TAX";

  dnMap["dd"] = "DD";
  mdMap["dd"] = false;
  if ( dshdlg->ddmoType->isChecked() )
    mdMap["dd"] = true;
  
  dnMap["ff"] = "FF";
  mdMap["ff"] = false;
  if ( dshdlg->ffmoType->isChecked() )
    mdMap["ff"] = true;
 
  dnMap["fxfx"] = "FX";
  if ( dshdlg->fx01Type->isChecked() )
    dnMap["fxfx"] = "FX_1";
  
  dnMap["ff_911"] = "FG";
  if ( dshdlg->fg01Type->isChecked() )
    dnMap["ff_911"] = "FG_1";
  if ( dshdlg->fg10Type->isChecked() )
    dnMap["ff_911"] = "FG_10";
  
  dnMap["sss"] = "SA";
  if ( dshdlg->sdType->isChecked() )
    dnMap["sss"] = "SD";
  if ( dshdlg->emType->isChecked() )
    dnMap["sss"] = "EM";
  
  dnMap["h"] = "HL";
  dnMap["N"] = "NN";
  dnMap["a"] = "AA";
  dnMap["ww"] = "WW";
  dnMap["Nh"] = "NH";
  dnMap["Cl"] = "CL";
  dnMap["Cm"] = "CM";
  dnMap["Ch"] = "CH";
  dnMap["TwTwTw"] = "TW";
  dnMap["VV"] = "VV";
  dnMap["W1"] = "W1";
  dnMap["W2"] = "W2";
  dnMap["HwaHwa"] = "HWA";
  dnMap["PwaPwa"] = "PWA";
  dnMap["dw1dw1"] = "DW1";
  dnMap["Pw1Pw1"] = "PW1";
  dnMap["Hw1Hw1"] = "HW1";
  dnMap["s"] = "SG";
  dnMap["ds"] = "MDIR";
  dnMap["vs"] = "MSPEED";
  if ( listExist )
    ListOK();
}

void HqcMainWindow::ListOK() {
  if ( !dianaconnected ) {
    int dianaWarning = QMessageBox::warning(this, 
					    "Dianaforbindelse",
					    "Diana er ikke koplet til!"
					    "Ønsker du å kople til Diana?",
					    "Ja", 
					    "Nei");
    if ( dianaWarning == 0 ) {
      return;
    }
  }
  if ( !statSelect || statSelect->stlist.size() == 0 ) {
    QMessageBox::warning(this, 
			 "Stasjonsvalg", 
			 "Ingen stasjoner er valgt!\n"
			 "Minst en stasjon må velges", 
			  QMessageBox::Ok, 
			  Qt::NoButton);
    return; 
  }
  bool noTimes = TRUE;
  for ( int hour = 0; hour < 24; hour++ ) { 
    if ( (clkdlg->clk[hour]->isChecked()) )
      noTimes = FALSE;
  }
  if ( noTimes ) {
    QMessageBox::warning(this, 
			 "Tidspunktvalg", 
			 "Ingen tidspunkter er valgt!\n"
			 "Minst ett tidspunkt må velges", 
			  QMessageBox::Ok, 
			  Qt::NoButton);
    return; 
  }

  if ( wElement.isEmpty() ) {
    QMessageBox::warning(this, 
			 "Værelement", 
			 "Ingen værelement er valgt!\n"
			 "Værelement må velges", 
			  QMessageBox::Ok, 
			  Qt::NoButton);
    return; 
  }

  BusyIndicator busyIndicator;
  listExist = TRUE;
  remstList = stList;
  stList.clear();
  for ( QStringList::Iterator sit = statSelect->stlist.begin(); 
        sit != statSelect->stlist.end(); 
        ++sit ) {
    int ind = QString(*sit).stripWhiteSpace().find(' ');
    stList.push_back(QString(*sit).stripWhiteSpace().left(ind).toInt());
  }
  miutil::miTime stime;
  stime.setTime(miutil::miString(lstdlg->getStart().latin1()));
  miutil::miTime etime;
  etime.setTime(miutil::miString(lstdlg->getEnd().latin1()));
 
  int dateCol = 0;
  if ( isShPo && !isShSt )
    dateCol = 1;
  else if ( !isShPo && isShSt )
    dateCol = 2;
  else if ( isShPo && isShSt )
    dateCol = 3;

  int ncp;
  if ( !isShOr && !isShFl && !isShMo )
    ncp = 0;
  if ( isShOr && !isShFl && !isShMo )
    ncp = 1;
  if ( !isShOr && isShFl && !isShMo )
    ncp = 2;
  if ( isShOr && isShFl && !isShMo )
    ncp = 3;
  if ( !isShOr && !isShFl && isShMo )
    ncp = 4;
  if ( isShOr && !isShFl && isShMo )
    ncp = 5;
  if ( !isShOr && isShFl && isShMo )
    ncp = 6;
  if ( isShOr && isShFl && isShMo )
    ncp = 7;

  int maxOrder;
  int* porder;
  if ( wElement == "Lufttrykk" ) {
    porder = airPressOrder;
    maxOrder = NOPARAMAIRPRESS;
  }
  else if ( wElement == "Temperatur" ) {
    porder = tempOrder;
    maxOrder = NOPARAMTEMP;
  }
  else if ( wElement == "Nedbør" ) {
    porder = precOrder;
    maxOrder = NOPARAMPREC;
  }
  else if ( wElement == "Visuell" ) {
    porder = visualOrder;
    maxOrder = NOPARAMVISUAL;
  }
  else if ( wElement == "Sjøgang" ) {
    porder = waveOrder;
    maxOrder = NOPARAMWAVE;
  }
  else if ( wElement == "Synop" ) {
    porder = synopOrder;
    maxOrder = NOPARAMSYNOP;
  }
  else if ( wElement == "Klimastatistikk" ) {
    porder = klstatOrder;
    maxOrder = NOPARAMKLSTAT;
  }
  else if ( wElement == "Prioriterte parametere" ) {
    porder = priorityOrder;
    maxOrder = NOPARAMPRIORITY;
  }
  else if ( wElement == "Vind" ) {
    porder = windOrder;
    maxOrder = NOPARAMWIND;
  }
  else if ( wElement == "Pluviometerkontroll" ) {
    porder = pluOrder;
    maxOrder = NOPARAMPLU;
  }
  else if ( wElement == "Alt" ) {
    porder = order;
    maxOrder = NOPARAMALL;
  }

  if ( selPar.count() > 0 ) selPar.clear();
  int kk = 0;
  for ( int jj = 0; jj < maxOrder; jj++ ) {
    if ( jj >= maxOrder ) break;
    
    const QString sp =  QString(parMap[*(porder+jj)].latin1());
    
    bool found = pardlg->plb->item(jj)->isSelected();
    if ( (found && pardlg->markPar->isChecked()) ||
	 (!found && pardlg->noMarkPar->isChecked()) ||
	 pardlg->allPar->isChecked() ) {
      selParNo[kk] = *(porder + jj);
      selPar.append(QString(sp));
      kk++;
    }
    noSelPar = kk;
  }

  if ( lstdlg->allTypes->isChecked() )
    isShTy = true;
  else
    isShTy = false;

  if ( lity == erLi || lity == erSa || lity == daLi) {
    metty = tabList;
    eTable(stime, 
	   etime, 
	   remstime, 
	   remetime, 
	   lity, 
	   remLity, 
	   metty, 
	   wElement,
	   selParNo, 
	   datalist, 
	   modeldatalist, 
	   slist,
	   dateCol,
	   ncp,
	   isShTy,
	   userName);
  }
  else if ( lity == alLi) {
    metty = tabList;
    eTable(stime, 
	   etime, 
	   remstime, 
	   remetime, 
	   daLi, 
	   remLity, 
	   metty, 
	   wElement,
	   selParNo, 
	   datalist, 
	   modeldatalist, 
	   slist,
	   dateCol,
	   ncp,
	   isShTy,
	   userName);
    eTable(stime, 
	   etime, 
	   remstime, 
	   remetime, 
	   erLi, 
	   remLity, 
	   metty, 
	   wElement,
	   selParNo, 
	   datalist, 
	   modeldatalist, 
	   slist,
	   dateCol,
	   ncp,
	   isShTy,
	   userName);
  }
  
  //  send parameter names to ts dialog
  emit newParameterList(selPar);
  if ( lity != erLi && lity != erSa  ) {
    sendAnalysisMessage();
    cerr << "HQC send times to diana" << endl;
    sendTimes();
  }
}

void HqcMainWindow::stationsInList() {
  std::list<kvalobs::kvStation>::const_iterator it=slist.begin();
  int stnr = it->stationID();
  for(;it!=slist.end(); it++){
    if ( it->stationID() != stnr ) {
      sillist.push_back(*it);
    }
    stnr = it->stationID();
  } 
}

void HqcMainWindow::TimeseriesOK() {
  miutil::miTime stime;
  miutil::miTime etime;
  vector<miutil::miString> parameter;
  vector<POptions::PlotOptions> plotoptions;
  vector<int> parameterIndex;
  vector<int> stationIndex;

  tsdlg->getResults(parameter,stime,etime,stationIndex,plotoptions);

  // make timeseries
  TimeSeriesData::tsList tslist;

  int nTypes = tsdlg->obsCheckBox->isChecked() + tsdlg->modCheckBox->isChecked();  
    
  for ( int ip = 0; ip < parameter.size(); ip++ ) {
    std::list<kvalobs::kvParam>::const_iterator it=plist.begin();
    for(;it!=plist.end(); it++){
      if ( it->name() == parameter[ip] ){
	parameterIndex.push_back(it->paramID());
	break;
      }
    }

    TimeSeriesData::TimeSeries tseries;
    tseries.stationid(stationIndex[ip]);  // set stationid
    tseries.paramid(parameterIndex[ip]);     // set parameter-number

    tseries.plotoptions(plotoptions[ip]); // set plotoptions for this curve
    if (tsdlg->modCheckBox->isChecked() && ( nTypes == 1 || ip%nTypes != 0) ) {
      for ( int i = 0; i < modeldatalist.size(); i++) { // fill data
	if ( modeldatalist[i].stnr == stationIndex[ip] &&
	     modeldatalist[i].otime >= stime &&
	     modeldatalist[i].otime <= etime ){
	  tseries.add(TimeSeriesData::Data(modeldatalist[i].otime,
					   modeldatalist[i].orig[parameterIndex[ip]]));
	}
      }
      if(tseries.dataOK()) {
	tslist.push_back(tseries);
      }
    }
    if ( tsdlg->obsCheckBox->isChecked() && (nTypes == 1 || ip%nTypes == 0) ) {
      for ( int i = 0; i < datalist.size(); i++) { // fill data
	if ( datalist[i].stnr() == stationIndex[ip] &&
	     datalist[i].otime() >= stime &&
             datalist[i].otime() <= etime &&
	     datalist[i].otime().min() == 0 ) {
	  if ( datalist[i].corr(parameterIndex[ip]) > -32766.0 )
	    tseries.add(TimeSeriesData::Data(datalist[i].otime(),
					     datalist[i].corr(parameterIndex[ip])));
	}
      }
      if(tseries.dataOK()) {
	tslist.push_back(tseries);
      }
    }
    else if (tsdlg->modCheckBox->isChecked() && tsdlg->obsCheckBox->isChecked() ) {
      for ( int i = 0; i < modeldatalist.size(); i++) { // fill data
	if ( modeldatalist[i].stnr == stationIndex[ip] &&
	     modeldatalist[i].otime >= stime &&
	     modeldatalist[i].otime <= etime ){
	  tseries.add(TimeSeriesData::Data(modeldatalist[i].otime,
					  datalist[i].corr(parameterIndex[ip])
					  - modeldatalist[i].orig[parameterIndex[ip]]));
	}
      }
      if(tseries.dataOK()) {
	tslist.push_back(tseries);
      }
    }
  }

  if(tslist.size() == 0){
    tspdialog->hide();
    tsVisible = FALSE;
    return;
  }

  // finally: one complete plot-structure
  TimeSeriesData::TSPlot tsplot;
  POptions::PlotOptions poptions;
  poptions.time_types.push_back(POptions::T_DATE);
  poptions.time_types.push_back(POptions::T_DAY);
  poptions.time_types.push_back(POptions::T_HOUR);

  poptions.name= "HQC Tidsserie";
  poptions.fillcolour= POptions::Colour("white");
  poptions.linecolour= POptions::Colour("black");
  poptions.linewidth= 1;  
  tsplot.plotoptions(poptions); // set global plotoptions

  tsplot.tserieslist(tslist);      // set list of timeseries
  
  tspdialog->prepare(tsplot);
  tspdialog->show();
  tsVisible = TRUE;
}

void HqcMainWindow::stationOK() {
  int noStat = 0;
  for ( QStringList::Iterator sit = listStatName.begin(); 
	sit != listStatName.end(); 
	++sit ) {
    noStat++;
  }
  lstdlg->removeAllStatFromListbox();
  statSelect = new StationSelection(listStatNum, 
				    listStatName,
				    listStatHoh,
				    listStatType,
				    listStatFylke,
				    listStatKommune,
				    listStatWeb,
				    listStatPri,
				    noStat,
				    lstdlg->aaType->isChecked(),
				    lstdlg->afType->isChecked(),
				    lstdlg->alType->isChecked(),
				    lstdlg->avType->isChecked(),
				    lstdlg->aoType->isChecked(),
				    lstdlg->aeType->isChecked(),
				    lstdlg->mvType->isChecked(),
				    lstdlg->mpType->isChecked(),
				    lstdlg->mmType->isChecked(),
				    lstdlg->msType->isChecked(),
				    lstdlg->fmType->isChecked(),
				    lstdlg->nsType->isChecked(),
				    lstdlg->ndType->isChecked(),
				    lstdlg->noType->isChecked(),
				    lstdlg->piType->isChecked(),
				    lstdlg->ptType->isChecked(),
				    lstdlg->vsType->isChecked(),
				    lstdlg->vkType->isChecked(),
				    lstdlg->vmType->isChecked(),
				    lstdlg->allType->isChecked(),
				    lstdlg->oslCoun->isChecked(),
				    lstdlg->akeCoun->isChecked(),
				    lstdlg->ostCoun->isChecked(),
				    lstdlg->hedCoun->isChecked(),
				    lstdlg->oppCoun->isChecked(),
				    lstdlg->busCoun->isChecked(),
				    lstdlg->vefCoun->isChecked(),
				    lstdlg->telCoun->isChecked(),
				    lstdlg->ausCoun->isChecked(),
				    lstdlg->veaCoun->isChecked(),
				    lstdlg->rogCoun->isChecked(),
				    lstdlg->horCoun->isChecked(),
				    lstdlg->sogCoun->isChecked(),
				    lstdlg->morCoun->isChecked(),
				    lstdlg->sorCoun->isChecked(),
				    lstdlg->ntrCoun->isChecked(),
				    lstdlg->norCoun->isChecked(),
				    lstdlg->troCoun->isChecked(),
				    lstdlg->finCoun->isChecked(),
				    lstdlg->svaCoun->isChecked(),
				    lstdlg->allCoun->isChecked(),
				    lstdlg->webReg->isChecked(),
				    lstdlg->priReg->isChecked(),
				    noInfo,
				    &otpList);

  connect(statSelect,
	  SIGNAL(stationAppended(QString)), 
	  lstdlg, 
	  SLOT(appendStatInListbox(QString)));
  connect(statSelect,
	  SIGNAL(stationRemoved(QString)), 
	  lstdlg, 
	  SLOT(removeStatFromListbox(QString)));
  statSelect->show();
}

void HqcMainWindow::errListMenu() {
  lity = erLi;
  lstdlg->hideAll();
  listMenu();
}

void HqcMainWindow::errLogMenu() {
  lity = erLo;
  lstdlg->hideAll();
  listMenu();
}

void HqcMainWindow::allListMenu() {
  lity = alLi;
  lstdlg->hideAll();
  listMenu();
}
void HqcMainWindow::dataListMenu() {
  lity = daLi;
  lstdlg->hideAll();
  listMenu();
}

void HqcMainWindow::errLisaMenu() {
  lity = erSa;
  lstdlg->hideAll();
  listMenu();
}

void HqcMainWindow::textDataMenu() {
  if ( txtdlg->isVisible() ) {
    txtdlg->hide();
  }
  else {
    txtdlg->show();
  }
}

void HqcMainWindow::rejectedMenu() {
  if ( rejdlg->isVisible() ) {
    rejdlg->hide();
  }
  else {
    rejdlg->show();
  }
}

inline QString dateStr_( const QDateTime & dt )
{
  QString ret = dt.toString( Qt::ISODate );
  ret[ 10 ] = ' ';
  return ret;
}

void HqcMainWindow::textDataOK() {
  int stnr = txtdlg->stnr;
  QDate todt = (txtdlg->dtto).date();
  QTime toti = (txtdlg->dtto).time();
  miutil::miTime dtto(todt.year(), todt.month(), todt.day() ,toti.hour(), 0, 0);
  QDate fromdt = (txtdlg->dtfrom).date();
  QTime fromti = (txtdlg->dtfrom).time();
  miutil::miTime dtfrom(fromdt.year(), fromdt.month(), fromdt.day() ,fromti.hour(), 0, 0);
  WhichDataHelper whichData;
  whichData.addStation(stnr, dtfrom, dtto);
  //  KvObsDataList txlist;
  GetTextData textDataReceiver(this);
  if(!KvApp::kvApp->getKvData(textDataReceiver, whichData)){
    cerr << "Finner ikke  textdatareceiver!!" << endl;
  }
  TextData* txtDat = new TextData(txtList, parMap);
  txtDat->show();
  if ( txtdlg->isVisible() ) {
    txtdlg->hide();
  }
  else {
    txtdlg->show();
  }
}

void HqcMainWindow::rejectedOK() {
  CKvalObs::CService::RejectDecodeInfo rdInfo;
  rdInfo.fromTime = dateStr_( rejdlg->dtfrom );
  rdInfo.toTime = dateStr_( rejdlg->dtto );
  cout << rdInfo.fromTime << " <-> " << rdInfo.toTime << endl;
  kvservice::RejectDecodeIterator rdIt;
  bool result = kvservice::KvApp::kvApp->getKvRejectDecode( rdInfo, rdIt );
  if ( result ) {
    string decoder = "comobs";

    kvalobs::kvRejectdecode reject;
    while ( rdIt.next( reject ) ) {
      
      if ( reject.decoder().substr( 0, decoder.size() ) != decoder ) {
        continue;
      }
      if ( reject.comment() == "No decoder for SMS code <12>!" ) {
        continue;
      }

      cout << reject.tbtime() << " " << reject.message() << " " << reject.comment() << reject.decoder() << endl;
      rejList.push_back(reject);
    }
  } else {
    cout << "No rejectdecode!" << endl;

  }
  Rejects* rejects = new Rejects(rejList);
  rejects->show();
  if ( rejdlg->isVisible() ) {
    rejdlg->hide();
  }
  else {
    rejdlg->show();
  }
}

void HqcMainWindow::showWatchRR()
{
  MDITabWindow * current = dynamic_cast<MDITabWindow *>( ws->activeWindow() );
  kvalobs::kvData data;

  if ( current ) {
    if ( current->erl )
      data = current->erl->getKvData();
    else if ( current->dtt )
      data = current->dtt->getKvData();
  }

  WatchRR::RRDialog * rrd = WatchRR::RRDialog::getRRDialog( data, slist, this, Qt::Window );
  if ( rrd ) {
    rrd->setReinserter( reinserter );
    rrd->show();
  }
}

void HqcMainWindow::showWeather()
{
  MDITabWindow * current = dynamic_cast<MDITabWindow *>( ws->activeWindow() );
  kvalobs::kvData data;
  if ( current ) {
    if ( current->erl ) {
      data = current->erl->getKvData();
    }
  }
  Weather::WeatherDialog * wtd = Weather::WeatherDialog::getWeatherDialog( data, slist, this, Qt::Window );
  if ( wtd ) {
    wtd->setReinserter( reinserter );
    wtd->show();
  }
}

void HqcMainWindow::listMenu() {
  if ( lstdlg->isVisible() ) {
    lstdlg->hideAll();
  } else {
    miutil::miTime mx = miutil::miTime::nowTime();
    int noDays = lstdlg->toTime->time().dayOfYear() - lstdlg->fromTime->time().dayOfYear();
    if ( noDays < 0 ) noDays = 365 + noDays;

    if ( noDays < 27 ) {
      mx.addMin(-1*mx.min());
      mx.addHour(1);
      lstdlg->toTime->setMax(mx);
      lstdlg->toTime->setTime(mx);
      lstdlg->toTime->setMax(mx);
      lstdlg->toTime->setTime(mx);
    }

    lstdlg->showAll();
  }
}

void HqcMainWindow::clockMenu() {
  if ( clkdlg->isVisible() ) {
    clkdlg->hideAll();
  } else {
    clkdlg->showAll();
  }
}

void HqcMainWindow::dianaShowMenu() {
  if ( dshdlg->isVisible() ) {
    dshdlg->hideAll();
  } else {
    dshdlg->showAll();
  }
}

void HqcMainWindow::paramMenu() {
  if ( pardlg->isVisible() ) {
    pardlg->hideAll();
  } else {
    pardlg->showAll();
  }
}

void HqcMainWindow::timeseriesMenu() {
  if ( tsdlg->isVisible() ) {
    tsdlg->hideAll();
  } else {
    tsdlg->showAll();
  }
}

void HqcMainWindow::clk() {
  clkdlg->showAll();
}

void HqcMainWindow::dsh() {
  dshdlg->showAll();
}

void HqcMainWindow::startKro() {
  system("firefox kro/cgi-bin/start.pl &");
}

HqcMainWindow::~HqcMainWindow()
{
}

void HqcMainWindow::listData(int index, 
			     int& stnr, 
			     miutil::miTime& obstime,
			     double* orig, 
			     int* flag, 
			     double* corr,
			     double* morig,
			     string* controlinfo,
			     string* useinfo,
			     string* cfailed,
			     int* typeId,
			     int& showTypeId,
			     int& typeIdChanged) {
  // List all data, all parameters at one station and one time
  miutil::miTime modeltime;
  int modelstnr;
  stnr = datalist[index].stnr();
  obstime = datalist[index].otime();
  showTypeId = datalist[index].showTypeId();
  typeIdChanged = datalist[index].typeIdChanged();
  int hour = obstime.hour();
  for ( int i = 0; i < NOPARAM; i++ ) {
    typeId[i] = datalist[index].typeId(i);
    orig[i] = datalist[index].orig(i);
    flag[i] = datalist[index].flag(i);
    corr[i] = datalist[index].corr(i);
    controlinfo[i] = datalist[index].controlinfo(i);
    std::cout << '<' << controlinfo[i].c_str() <<  '>' << std::endl;
    useinfo[i] = datalist[index].useinfo(i);
    cfailed[i] = datalist[index].cfailed(i);
    morig[i] = -32767.0;
  }

  for ( int j = 0; j < modeldatalist.size(); j++ ) {
    modeltime = modeldatalist[j].otime;
    modelstnr = modeldatalist[j].stnr;
    if ( modelstnr == stnr && modeltime == obstime ) {
      for ( int i = 0; i < NOPARAMMODEL; i++ ) {
	morig[modelParam[i]] = modeldatalist[j].orig[modelParam[i]];
      }
    }	    
  }
}
/*
Convert to "Diana-value" of range check flag 
*/
int HqcMainWindow::numCode1(int nib) {
  int code = 9;
  if ( nib == 0 )
    code = 0;
  else if ( nib == 1 )
    code = 1;
  else if ( nib == 2 || nib == 3 )
    code = 2;
  else if ( nib == 4 || nib == 5 )
    code = 3;
  else if ( nib == 6 )
    code = 9;
  return code;
}

/*
Convert to "Diana-value" of consistency check flag 
*/
int HqcMainWindow::numCode2(int nib) {
  int code = 9;
  if ( nib == 0 )
    code = 0;
  else if ( nib == 1 )
    code = 1;
  else if ( nib >= 2 && nib <= 7 )
    code = 8;
  else if ( nib == 10 || nib == 11 )
    code = 7;
  else if ( nib == 13 )
    code = 9;
  return code;
}

/*
Convert to "Diana-value" of prognostic space control flag 
*/
int HqcMainWindow::numCode3(int nib) {
  int code = 9;
  if ( nib == 0 )
    code = 0;
  else if ( nib == 1 )
    code = 1;
  else if ( nib == 2 || nib == 3 )
    code = 2;
  else if ( nib == 4 || nib == 5 )
    code = 3;
  else if ( nib == 6 )
    code = 5;
  return code;
}

/*
Convert to "Diana-value" of step check flag 
*/
int HqcMainWindow::numCode4(int nib) {
  int code = 9;
  if ( nib == 0 )
    code = 0;
  else if ( nib == 1 )
    code = 1;
  else if ( nib == 2 || nib == 3 )
    code = 2;
  else if ( nib >= 4 && nib <= 7 )
    code = 8;
  else if ( nib == 9 || nib == 10 )
    code = 7;
  return code;
}

/*
Convert to "Diana-value" of timeseries adaption flag 
*/
int HqcMainWindow::numCode5(int nib) {
  int code = 9;
  if ( nib == 0 )
    code = 0;
  else if ( nib == 1 || nib == 2 )
    code = 5; 
  else if ( nib == 3 )
    code = 3;
  return code;
}

/*
Convert to "Diana-value" of statistics control flag 
*/
int HqcMainWindow::numCode6(int nib) {
  int code = 9;
  if ( nib == 0 )
    code = 0;
  else if ( nib == 1 )
    code = 1;
  else if ( nib == 2 )
    code = 3;
  return code;
}

/*
Convert to "Diana-value" of climatology control flag 
*/
int HqcMainWindow::numCode7(int nib) {
  int code = 9;
  if ( nib == 0 )
    code = 0;
  else if ( nib == 1 )
    code = 1;
  else if ( nib == 2 )
    code = 3;
  else if ( nib == 3 )
    code = 7;
  return code;
}

/*
Convert to "Diana-value" of HQC flag 
*/
int HqcMainWindow::numCode8(int nib) {
  int code = 9;
  if ( nib <  10 )
    code = nib;
  else
    code = 9;
  return code;
}

/*
 Calculate the 5-digit flag-code to be shown in Diana
*/
int HqcMainWindow::getShowFlag(kvalobs::kvDataFlag controlInfo) {

  // Find flags from the different checks

  int nib1  =controlInfo.flag(1);
  int nib2  =controlInfo.flag(2);
  int nib3  =controlInfo.flag(4);
  int nib4  =controlInfo.flag(3);
  int nib5  =controlInfo.flag(7);
  int nib6  =controlInfo.flag(9);
  int nib7  =controlInfo.flag(11);
  int nib8  =controlInfo.flag(10);
  int nib9  =controlInfo.flag(12);
  int nib10 =controlInfo.flag(15);
  // Decode flags

  int nc1 = numCode1(nib1); // Range check
  int nc2 = numCode2(nib2); // Formal Consistency check
  int nc8 = numCode2(nib8); // Climatologic Consistency check
  // Use the largest value from these checks
  nc1 = nc1 > nc2 ? nc1 : nc2;
  nc1 = nc1 > nc8 ? nc1 : nc8;
  nc2 = numCode3(nib3); //Prognostic space control
  int nc3 = numCode4(nib4); //Step check
  int nc4 = numCode5(nib5); //Timeseries adaption
  int nc5 = numCode6(nib6); //Statistics control
  int nc6 = numCode7(nib7); //Climatology control
  // Use the largest value from the three last checks
  nc4 = nc4 > nc5 ? nc4 : nc5;
  nc4 = nc4 > nc6 ? nc4 : nc6;
  if ( nib9 > 1 )
    nc4 = nc4 > 6 ? nc4 : 6;
  nc5 = numCode8(nib10);
  int nc = 10000*nc1 + 1000*nc2 + 100*nc3 + 10*nc4 + nc5;

  return nc;
}

bool HqcMainWindow::timeFilter(int hour) {
  if ( clkdlg->clk[hour]->isChecked() )
    return TRUE;
  return FALSE;
}

bool HqcMainWindow::hqcTypeFilter(const int& typeId, int environment, int stnr) {
  //  if ( typeId == -1 || typeId == 501 ) return FALSE;
  if ( typeId == -1 ) return FALSE;
  if ( lstdlg->webReg->isChecked() || lstdlg->priReg->isChecked() ) return TRUE;
  int atypeId = typeId < 0 ? -typeId : typeId;
  if (  lstdlg->allType->isChecked() ) return TRUE;
  if ( environment == 1 && atypeId == 311 && lstdlg->afType->isChecked() ) return TRUE;
  if ( (environment == 8 && (atypeId == 3 || atypeId == 311 || atypeId == 412)) || (atypeId == 330 || atypeId == 342) && lstdlg->aaType->isChecked() ) return TRUE;
  if ( environment == 2 && atypeId == 3 && lstdlg->alType->isChecked() ) return TRUE;
  if ( environment == 12 && atypeId == 3 && lstdlg->avType->isChecked() ) return TRUE;
  if ( atypeId == 410 && lstdlg->aoType->isChecked() ) return TRUE;
  if ( environment == 7 && atypeId == 11 && lstdlg->mvType->isChecked() ) return TRUE;
  if ( environment == 5 && atypeId == 11 && lstdlg->mpType->isChecked() ) return TRUE;
  if ( environment == 4 && atypeId == 11 && lstdlg->mmType->isChecked() ) return TRUE;
  if ( environment == 6 && atypeId == 11 && lstdlg->msType->isChecked() ) return TRUE;
  if ( (atypeId == 4 || atypeId == 404) && lstdlg->piType->isChecked() ) return TRUE;
  if ( (atypeId == 4 || atypeId == 404)&& lstdlg->ptType->isChecked() ) return TRUE;
  if ( atypeId == 302 && lstdlg->nsType->isChecked() ) return TRUE;
  if ( environment == 9 && atypeId == 402 && lstdlg->ndType->isChecked() ) return TRUE;
  if ( environment == 10 && atypeId == 402 && lstdlg->noType->isChecked() ) return TRUE;
  if ( (atypeId == 1 || atypeId == 6 || atypeId == 312 || atypeId == 412) & lstdlg->vsType->isChecked() ) return TRUE;
  if ( environment == 3 && atypeId == 412 && lstdlg->vkType->isChecked() ) return TRUE;
  if ( (atypeId == 306 || atypeId == 308 || atypeId == 412) && lstdlg->vmType->isChecked() ) return TRUE;
  if ( atypeId == 2 && lstdlg->fmType->isChecked() ) return TRUE;
  return FALSE;
}

bool HqcMainWindow::typeIdFilter(int stnr, int typeId, int sensor, miutil::miTime otime, int par) {
  bool tpf = false;
  //
  // Midlertidig spesialtilfelle for Venabu!!!!!!!!!!!!!!!
  //
  if ( stnr == 13420 && par == 112 ) {
    if ( typeId == 330 )
      return false;
    else if ( typeId == 308 )
      return true;
  }
  //
  // Midlertidig spesialtilfelle for Rygge!!!!!!!!!!!!!!!
  //
  if ( stnr == 17150 && (par == 109 || par == 110)) {
    if ( typeId == -342 )
      return false;
    else if ( typeId == 342 )
      return true;
  }
  //
  // Midlertidig spesialtilfelle for Jan Mayen!!!!!!!!!!!!!!!
  //
  if ( stnr == 99950 ) {
    int hr = otime.hour();
    int dg = otime.day();
    int mn = otime.month();
    int ar = otime.year();
    if ( ar == 2008 && mn == 7 && dg == 28 && (( hr < 10 && typeId == 3) || ( hr >= 10 && typeId == 330)) )
      return true;
    else if ( ar == 2008 && mn == 7 && dg == 28 && ( hr >= 10 && typeId == 3 ) )
      return false;
  }
  //Spesialtilfelle slutt
  //
  // Midlertidig spesialtilfelle for Fokstugu!!!!!!!!!!!!!!!
  //
  if ( stnr == 16610 ) {
    int hr = otime.hour();
    int dg = otime.day();
    int mn = otime.month();
    int ar = otime.year();
    if ( ar == 2008 && mn == 10 && dg == 9 && (( hr < 16 && typeId == 3) || ( hr >= 16 && typeId == 330)) )
      return true;
    else if ( ar == 2008 && mn == 10 && dg == 9 && ( hr >= 16 && typeId == 3 ) )
      return false;
  }
  //Spesialtilfelle slutt
  //  if ( typeId == -404 || (typeId == -342 && (par == 109 || par == 110))) return false;
  if ( par == 173 && typeId < 0 ) return false;
  if ( typeId == -404 ) return false;
  if ( stnr == 4780 && typeId == 1 ) return true;
  if ( (stnr == 68860 ) && typeId == -4 ) return false;
  if ( typeId < 0 && !((stnr == 18700 || stnr == 50540 || stnr == 90450 || stnr == 99910) && par == 109) ) return true;
  for ( vector<currentType>::iterator it = currentTypeList.begin(); it != currentTypeList.end(); it++) {
    if ( stnr == (*it).stnr && 
	 abs(typeId) == (*it).cTypeId &&
	 sensor == (*it).cSensor && 
	 par == (*it).par && 
	 otime.date() >= (*it).fDate && 
	 otime.date() <= (*it).tDate ) {
      tpf = true;
      break;
    }
  }
  return tpf;
}

/*!
 Read the data table in the Kvalobs database
*/
void HqcMainWindow::readFromData(const miutil::miTime& stime, 
				 const miutil::miTime& etime, 
				 listType lity) {
  BusyIndicator busy();

  bool result;
  QTime t;
  t.start();
  cerr << "Tid for readFromTypeIdFile = " << t.elapsed() << endl;
  t.restart();
  WhichDataHelper whichData;
  for ( int i = 0; i < stList.size(); i++ ) {
    whichData.addStation(stList[i], stime, etime);
    checkTypeId(stList[i]);
  }
  cerr << "Tid for whichData.addStation og checkTypeId = " << t.elapsed() << endl;
 
  t.restart();
  
  KvObsDataList ldlist;// = GetData::datalist;
  GetData dataReceiver(this);
  if(!KvApp::kvApp->getKvData(dataReceiver, whichData)){
    cerr << "Finner ikke  datareceiver!!" << endl;
  }
  else {
    cerr << "Datareceiver OK!" << endl;
  }
  cerr << "Tid for new getKvData (hele readfromdata) = " << t.elapsed() << endl;
  /*
  KvObsDataList txlist;// = GetData::datalist;
  GetTextData textDataReceiver(this);
  if(!KvApp::kvApp->getKvData(textDataReceiver, whichData)){
    cerr << "Finner ikke  textdatareceiver!!" << endl;
  }
  */
}


/*!
 Read the modeldata table in the Qualobs database
*/

void HqcMainWindow::readFromModelData(const miutil::miTime& stime, 
				      const miutil::miTime& etime) {
  bool result;
  
  mdlist.erase(mdlist.begin(),mdlist.end());
  modeldatalist.reserve(131072);
  modeldatalist.clear();  
  QTime t;
  t.start();
  WhichDataHelper whichData;
  for ( int i = 0; i < stList.size(); i++ ) {
    whichData.addStation(stList[i], stime, etime);
  }
  cerr << "Tid for whichData.addStation og checkTypeId = " << t.elapsed() << endl;
  t.restart();

  if(!KvApp::kvApp->getKvModelData(mdlist, whichData))
    cerr << "Can't connect to modeldata table!" << endl;
  cerr << "Tid for getKvModelData = " << t.elapsed() << endl;
  t.restart();
  modDatl mtdl;
  for ( int ip = 0; ip < NOPARAMMODEL; ip++) {
    mtdl.orig[modelParam[ip]] = -32767.0;
  }
  miutil::miTime protime("1800-01-01 00:00:00");
  int prstnr = 0;
  CIModelDataList it=mdlist.begin();
  while ( it != mdlist.end() ) {
    int stnr = it->stationID();
    miutil::miTime otime = (it->obstime());
    mtdl.otime = otime;
    mtdl.stnr = stnr;
    mtdl.orig[it->paramID()] = it->original();
    protime = otime;
    prstnr = stnr;
    otime = (++it)->obstime();
    stnr = it->stationID();
    if ( otime != protime || ( otime == protime && stnr != prstnr)) {
      modeldatalist.push_back(mtdl);
      for ( int ip = 0; ip < NOPARAMMODEL; ip++) {
	mtdl.orig[modelParam[ip]] = -32767.0;
      }
    }
  }
  cerr << "Tid for resten av readFromModelData = " << t.elapsed() << endl;
}

/*!
 Read the station table in the kvalobs database
*/
void HqcMainWindow::readFromStation() {
  if (!KvApp::kvApp->getKvStations(slist)) {
    cerr << "Can't connect to station table!" << endl;
    int noBase = QMessageBox::warning(this, 
				      "Kvalobs", 
				      "Kvalobsdatabasen er ikke tilgjengelig,\n"
				      "vil du avslutte?", 
				      "Ja",
				      "Nei", 
				      "" );
    if ( noBase == 0 )
      exit(1);
  }
  std::list<kvalobs::kvStation>::const_iterator it=slist.begin();
  for(;it!=slist.end(); it++){
    stnrList.push_back(it->stationID());
  }
}
/*!
 Read the obs_pgm table in the kvalobs database
*/

void HqcMainWindow::readFromObsPgm() {
  if (!KvApp::kvApp->getKvObsPgm(obsPgmList, statList, false))
    cerr << "Can't connect to obs_pgm table!" << endl;
}

/*!
 Read the typeid file
*/

void HqcMainWindow::checkTypeId(int stnr) {
  for(CIObsPgmList obit=obsPgmList.begin();obit!=obsPgmList.end(); obit++){
    int stationId = obit->stationID();
    if ( stationId == stnr ) {
      int par = obit->paramID();
      miutil::miDate fDate = obit->fromtime().date();
      miutil::miDate tDate = obit->totime().date();
      int level  = obit->level();
      int sensor = 0;
      int typeId = obit->typeID();
      crT.stnr = stationId;
      crT.par = par;
      crT.fDate = fDate;
      crT.tDate = tDate;
      crT.cLevel = level;
      crT.cTypeId = typeId;
      currentTypeList.push_back(crT);
    }
  }
}

/*!
 Read the station file, this must be done after the station table in the database is read
*/

void HqcMainWindow::readFromStationFile(int statCheck) {
  
  QString path = QString(getenv("HQCDIR"));
  if ( path.isEmpty() ) {
    cerr << "Intet environment" << endl;
    exit(1);
  }
  QString stationFile = path + "/etc/kvhqc/hqc_stations";
  QFile stations(stationFile);
  stations.open(QIODevice::ReadOnly);
  Q3TextStream stationStream(&stations);
  int i = 0;
  int prevStnr = 0;
  while ( stationStream.atEnd() == 0 ) {
    QString statLine = stationStream.readLine();
    QString qwert = statLine.mid(64,3);
    int stnr = statLine.left(7).toInt();
    int snr = 0;
    QString strSnr("    ");
    if ( stnr == prevStnr ) continue;
    std::list<kvalobs::kvStation>::const_iterator it=slist.begin();
    bool foundStation = FALSE;

    for(;it!=slist.end(); it++){
      if ( it->stationID() == stnr ) {
	foundStation = TRUE;
	if ( it->wmonr() > 0 )
	  strSnr = strSnr.setNum(it->wmonr());
	break;
      }
    }
    if ( foundStation ) {
      QString strStnr;
      QString strHoh;
      QString strEnv;
      strEnv = strEnv.setNum(it->environmentid());
      listStatName.append(it->name().cStr());
      listStatNum.append(strStnr.setNum(it->stationID()));
      listStatHoh.append(strHoh.setNum(it->height()));
      listStatType.append(strEnv);
      listStatFylke.append(statLine.mid(8,30).stripWhiteSpace());
      listStatKommune.append(statLine.mid(39,24).stripWhiteSpace());
      listStatWeb.append(strSnr);
      listStatPri.append(statLine.mid(67,4).stripWhiteSpace());
    }
    prevStnr = stnr;
  }
}

/*!
 Read the param table in the Qualobs database
*/

void HqcMainWindow::readFromParam() {

  // First, read parameter order from file

  QString path = QString(getenv("HQCDIR"));
  if ( path.isEmpty() ) {
    cerr << "Intet environment" << endl;
    exit(1);
  }
  
  QString orderFile = path + "/etc/kvhqc/paramorder";
  QFile paramOrder(orderFile);
  
  paramOrder.open(QIODevice::ReadOnly);
  Q3TextStream paramStream(&paramOrder);
  int i = 0;
  while ( paramStream.atEnd() == 0 ) {
    QString strLine = paramStream.readLine();
    if ( strLine == "[all]" ) {
      for ( int ii = 0; ii < NOPARAMALL; ii++){
	strLine = paramStream.readLine();
	order[ii] = strLine.toInt();
      }
    }
    else if ( strLine == "[airpress]" ) {
      for ( int ii = 0; ii < NOPARAMAIRPRESS; ii++){
	strLine = paramStream.readLine();
	airPressOrder[ii] = strLine.toInt();
      }
    }
    else if ( strLine == "[temperature]" ) {
      for ( int ii = 0; ii < NOPARAMTEMP; ii++){
	strLine = paramStream.readLine();
	tempOrder[ii] = strLine.toInt();
      }
    }
    else if ( strLine == "[prec]" ) {
      for ( int ii = 0; ii < NOPARAMPREC; ii++){
	strLine = paramStream.readLine();
	precOrder[ii] = strLine.toInt();
      }
    }
    else if ( strLine == "[visual]" ) {
      for ( int ii = 0; ii < NOPARAMVISUAL; ii++){
	strLine = paramStream.readLine();
	visualOrder[ii] = strLine.toInt();
      }
    }
    else if ( strLine == "[wave]" ) {
      for ( int ii = 0; ii < NOPARAMWAVE; ii++){
	strLine = paramStream.readLine();
	waveOrder[ii] = strLine.toInt();
      }
    }
    else if ( strLine == "[synop]" ) {
      for ( int ii = 0; ii < NOPARAMSYNOP; ii++){
	strLine = paramStream.readLine();
	synopOrder[ii] = strLine.toInt();
      }
    }
    else if ( strLine == "[klstat]" ) {
      for ( int ii = 0; ii < NOPARAMKLSTAT; ii++){
	strLine = paramStream.readLine();
	klstatOrder[ii] = strLine.toInt();
      }
    }
    else if ( strLine == "[priority]" ) {
      for ( int ii = 0; ii < NOPARAMPRIORITY; ii++){
	strLine = paramStream.readLine();
	priorityOrder[ii] = strLine.toInt();
      }
    }
    else if ( strLine == "[wind]" ) {
      for ( int ii = 0; ii < NOPARAMWIND; ii++){
	strLine = paramStream.readLine();
	windOrder[ii] = strLine.toInt();
      }
    }
    else if ( strLine == "[plu]" ) {
      for ( int ii = 0; ii < NOPARAMPLU; ii++){
	strLine = paramStream.readLine();
	pluOrder[ii] = strLine.toInt();
      }
    }
  }
  bool result;
  cerr.flags(ios::fixed);

  if(!KvApp::kvApp->getKvParams(plist))
    cerr << "Can't connect to param table!" << endl;
  std::list<kvalobs::kvParam>::const_iterator it=plist.begin();

  for(;it!=plist.end(); it++){
    parMap[it->paramID()] = it->name().cStr();
    listParName.append(it->name().cStr());
  }
}

/*!
 Find the name and position of a station from 
 a list extracted from the station table
*/
void HqcMainWindow::findStationInfo(int stnr, 
				    QString& name,
				    double& lat, 
				    double& lon, 
				    double& hoh,
				    int& snr,
				    int& env) {
  std::list<kvalobs::kvStation>::const_iterator it=slist.begin();
  for(;it!=slist.end(); it++){
    if ( it->stationID() == stnr ) {
      const char* cName = (it->name()).c_str();
      name = QString(cName);
      lat  = (it->lat());
      lon  = (it->lon());
      hoh  = (it->height());
      snr  = (it->wmonr());
      env  = (it->environmentid());
      break;
    }
  }
}

void HqcMainWindow::findStationPos(int stnr, 
				   double& lat, 
				   double& lon, 
				   double& hoh) {
  std::list<kvalobs::kvStation>::const_iterator it=slist.begin();
  for(;it!=slist.end(); it++){
    if ( it->stationID() == stnr ) {
      lat  = (it->lat());
      lon  = (it->lon());
      hoh  = (it->height());
      break;
    }
  }
}

void HqcMainWindow::tileHorizontal() {
  // primitive horizontal tiling
  QWidgetList windows = ws->windowList();
  if ( !windows.count() )
    return;
  int y = 0;
  if ( windows.count() == 1 ) {
    QWidget *window = windows.at(0);
    window->parentWidget()->setGeometry( 0, y, ws->width(), ws->height() );
    return;
  }
  else {
    int height[] = {0, 28 + ws->height() / 2, (ws->height() / 2) - 28} ;
    for ( int i = int(windows.count()) - 2; i < int(windows.count()); ++i ) {
      QWidget *window = windows.at(i);
      if ( window->windowState() == Qt::WindowMaximized ) {
	// prevent flicker
	window->hide();
	window->showNormal();
      }
      int preferredHeight = window->minimumHeight()+window->parentWidget()->baseSize().height();
      int actHeight = QMAX(height[i -int(windows.count()) + 3], preferredHeight);
      
      window->parentWidget()->setGeometry( 0, y, ws->width(), actHeight );
      y += actHeight;
    }
  }
}

MDITabWindow* HqcMainWindow::eTable(const miutil::miTime& stime,
				    const miutil::miTime& etime,
				    miutil::miTime& remstime,
				    miutil::miTime& remetime,
				    listType lity,
				    listType remLity,
				    mettType metty,
				    QString& wElement,
				    int* selParNo,
				    vector<model::KvalobsData>& datalist,
				    vector<modDatl>& modeldatalist,
				    list<kvStation>& slist,
				    int dateCol,
				    int ncp,
				    bool isShTy,
				    QString& userName)
{
  MDITabWindow* et = new MDITabWindow( ws, 
				       0, 
				       Qt::WDestructiveClose, 
				       stime, 
				       etime, 
				       remstime, 
				       remetime, 
				       lity, 
				       remLity, 
				       metty, 
				       wElement,
				       selPar,
				       noSelPar, 
				       selParNo,
				       datalist, 
				       modeldatalist,
				       slist,
				       dateCol,
				       ncp,
				       isShTy,
				       userName);
  ws->addWindow(et);
  connect( et, SIGNAL( message(const QString&, int) ), statusBar(), 
	   SLOT( message(const QString&, int )) );
  if ( lity == erLi || lity == erSa ) {
    et->setCaption("Feilliste");
  }
  else if ( lity == erLo )
    et->setCaption("Feillog");
  else if ( lity == daLi )
    et->setCaption("Dataliste");
  et->setIcon( QPixmap("/usr/local/etc/kvhqc/hqc.png") );
  et->show();
  tileHorizontal();
  vector<QString> stationList;
  int stnr=-1;
  for ( int i = 0; i < datalist.size(); i++) {
    QString name;
    double lat,lon,hoh;
    int env;
    int snr;
    if(stnr != datalist[i].stnr()){
      stnr = datalist[i].stnr();
      findStationInfo(stnr,name,lat,lon,hoh,snr,env);
      QString nrStr;
      nrStr = nrStr.setNum(stnr);
      QString statId = nrStr + " " + name;
      stationList.push_back(statId);
    }
  }

  emit newStationList(stationList);
  return et;
}

void HqcMainWindow::closeWindow()
{
  firstObs=true;
  cerr << "HqcMainWindow::closeWindow()\n";
    MDITabWindow* t = (MDITabWindow*)ws->activeWindow();
    if ( t )
	t->close();
    emit windowClose();
}


void HqcMainWindow::helpUse() {  
  QString path = QString(getenv("HQCDIR"));
  system("firefox https://dokit.met.no/klima/tools/qc/hqc-help &");
}

void HqcMainWindow::helpFlag() {  
  QString path = QString(getenv("HQCDIR"));
  system("firefox https://kvalobs.wiki.met.no/doku.php?id=kvalobs:kvalobs-flagg &");
}

void HqcMainWindow::helpParam() {
  QString path = QString(getenv("HQCDIR"));
  system("firefox https://kvalobs.wiki.met.no/doku.php?id=kvalobs:kvalobs-parametre_sortert_alfabetisk_etter_kode &");
}

void HqcMainWindow::about()
{
    QMessageBox::about( this, "Om Hqc",
			"Hqc er et program for manuell kvalitetskontroll av observasjoner.\n"
			"Programmet består av editerbare tabeller med observasjoner samt\n"
			"tidsseriediagram, og har forbindelse med Diana\n"
			"\n"
			"Programmet utvikles av\n "
			"Lisbeth Bergholt, FoU,\n "
			"Vegard Bønes, IT,\n "
			"Audun Christoffersen, FoU,\n "
			"Knut Johansen, Klima\n ");
}


void HqcMainWindow::initDiana()
{
  dianaconnected= false;

  cerr<< "Try to establish connection with diana.." << endl;

  miString type = "Diana";
    
      
  if(pluginB->clientTypeExist(type)){
    dianaconnected= true;
    cerr << "HQC connected to Diana" << endl;
    sendTimes();
    sendAnalysisMessage();
  } else {
    cerr << "No Diana clients found by HQC" << endl;
    return;
  }
  
}


// send one image to diana (with name)
void HqcMainWindow::sendImage(const miutil::miString name, const QImage& image)
{
  if (!dianaconnected) return;
  if (image.isNull()) return;
  
  QByteArray* a;
  QDataStream s(a, QIODevice::WriteOnly);
  s << image;
  
  miMessage m;
  m.command= qmstrings::addimage;
  m.description= "name:image";

  ostringstream ost;
  ost << name << ":";
  int n= a->count();
  for (int i=0; i<n; i++)
    ost << setw(7) << int(*a[i]);
  miutil::miString txt= ost.str();
  m.data.push_back(txt);
  cerr << "HQC sender melding" << endl;
  cerr <<"HQC: meldingen inneholder:"<< m.content() <<endl;
  pluginB->sendMessage(m);
}

// called when client-list changes
void HqcMainWindow::processConnect()
{
  initDiana();
}

void HqcMainWindow::cleanConnection()
{
  dianaconnected = false;
  firstObs = true;
  cout << "< DISCONNECTING >" << endl;
}

void HqcMainWindow::sendTimes(){

  //send times
  miMessage m;
  m.command= qmstrings::settime;
  m.commondesc = "datatype";
  m.common = "obs";
  m.description= "time";
  int n=datalist.size();
  for(int i=0;i<n;i++){
    m.data.push_back(datalist[i].otime().isoTime());
  }
  cerr << "HQC sender melding" << endl;
  //  cerr <<"HQC: meldingen inneholder:"<< m.content() <<endl;
  pluginB->sendMessage(m);
  
}

// processes incoming miMessages
void HqcMainWindow::processLetter(miMessage& letter)
{
  cerr << "HQC mottar melding : " << letter.command.c_str() << endl;
  if(letter.command == qmstrings::newclient) {
    firstObs = true;
    cerr << letter.to << " , " << letter.from << " , " << letter.clientType << " , " << letter.co << endl; 
    processConnect(); 
    hqcFrom = letter.to;
    hqcTo = letter.from;
  }
  else if (letter.command == "station" ) {
    const char* ccmn = letter.common.c_str();
    QString cmn = QString(ccmn);
    cerr << "Innkommende melding: statTimeReceived is emitted."  << endl;
    emit statTimeReceived(cmn);
  }
  
  else if(letter.command == qmstrings::timechanged){
    cerr <<"HQC: meldingen inneholder:"<< letter.content() <<endl;
    miutil::miTime newTime(letter.common);
    sendObservations(newTime,false);
  }
  
}


// send text to show in text-window in diana
/*
void HqcMainWindow::sendShowText(const miutil::miString site)
{
  if (!dianaconnected) return;

  miMessage m;
  m.command= qmstrings::showtext;
  m.description= DATASET_STATIONS+";POS:TEXT";
  miutil::miString data= site+":Dette er en tekst for posisjon " + site;
  m.data.push_back(data);
  cerr << "HQC:     command:" << m.command << endl;
  cerr << "HQC: description:" << m.description << endl;
  cerr << "HQC sender melding" << endl;
  pluginB->sendMessage(m);
}
*/

// send message to show ground analysis in Diana
bool  HqcMainWindow::sendAnalysisMessage() {
  
  //show analysis
  miMessage letter;
  letter.command=qmstrings::apply_quickmenu;
  letter.data.push_back("Hqc");
  letter.data.push_back("Bakkeanalyse");
  cerr << "HQC sender melding" << endl;
  cerr <<"HQC: meldingen inneholder:"<< letter.content() <<endl;
  pluginB->sendMessage(letter);
  
  dianaTime.setTime(miutil::miString("2000-01-01 00:00:00"));
  return true;
}

void HqcMainWindow::sendStation(int stnr)
{

  miMessage pLetter;
  pLetter.command = qmstrings::station;
  miutil::miString stationstr(stnr);
  pLetter.common = stationstr;
  cerr << "HQC sender melding" << endl;
  cerr <<"HQC: meldingen inneholder:"<< pLetter.content() <<endl;
  pluginB->sendMessage(pLetter);
  
}

void HqcMainWindow::aboutQt()
{
    QMessageBox::aboutQt( this, "Qt is a C++ toolkit for application development. " );
}


void HqcMainWindow::sendObservations(miutil::miTime time,
				     bool sendtime) 
{
  //no data -> return
  if(selPar.count() == 0) return;

  //just sent 
  if(dianaTime == time)
    return;

  dianaTime = time;

  if(sendtime){
    cerr << "Sendtime = true" << endl;
    miMessage tLetter;
    tLetter.command = qmstrings::settime;
    tLetter.commondesc = "time";
    tLetter.common = time.isoTime();
    cerr << "HQC sender melding" << endl;
    cerr <<"HQC: meldingen inneholder:"<< tLetter.content() <<endl;
    pluginB->sendMessage(tLetter);
  }
  miMessage pLetter;
  if ( firstObs ) 
    pLetter.command = qmstrings::init_HQC_params;
  else
    pLetter.command = qmstrings::update_HQC_params;
  pLetter.commondesc = "time,plottype";
  
  //finding parameter names and indexes
  vector<int> parIndex;
  QStringList parName;
  deque<bool> parSynop;
  deque<bool> parModel;
  deque<bool> parDiff;
  deque<bool> parProp;
  parSynop.insert(parSynop.begin(),selPar.count(),false);
  for ( int i=0; i<selPar.count(); ++i ) {
    parIndex.push_back(selParNo[i]);
    miutil::miString diPar = dianaName(selPar[i].latin1());
    if(diPar.exists()) {
      parName.append(diPar.cStr());
      parSynop[i] = true;
      parModel[i] = mdMap[diPar];
      parDiff[i]  = diMap[diPar];
      parProp[i]  = prMap[diPar];
    }
  }

  //  parName = selPar;
  miutil::miString synopDescription;
    synopDescription = "id,St.type,auto,lon,lat,";
  if ( !parName.isEmpty() )
    synopDescription += parName.join(",").latin1();
  miutil::miString enkelDescription = "id,St.type,auto,lon,lat,";
  if ( !parName.isEmpty() )
    enkelDescription += parName.join(",").latin1();

  vector<miutil::miString> synopData;
  vector<miutil::miString> enkelData;

  for ( int i = 0; i < datalist.size(); i++) { // fill data
    if ( datalist[i].otime() == time ){
      double lat,lon,h;
      miutil::miString str(datalist[i].stnr());
      QString name;
      int env;
      int snr;
      //      int typeId = datalist[i].typeId;
      int typeId = datalist[i].showTypeId();
      findStationInfo(datalist[i].stnr(),name,lat,lon,h,snr,env);
      str += ",";
      miutil::miString isAuto = "x";
      miutil::miString strType = hqcType(typeId, env);
      if ( strType.cStr()[0] == 'A' )
	isAuto = "a";
      else if ( strType.cStr()[0] == 'N' || strType.cStr()[0] == 'P' )
	isAuto = "n";
      if ( (isAuto == "a" && strType.cStr()[1] == 'A') || 
	   (strType.cStr()[0] == 'V' && strType.cStr()[1] != 'M') ||
	   (strType == "none")  )
    	str += "none";
      else if ( strType.cStr()[0] == 'P' )
	str += strType.cStr()[0];
      else
	str += strType.cStr()[1];
      str += ",";
      str += isAuto;
      str += ",";
     
      miutil::miString latstr(lat,4);
      miutil::miString lonstr(lon,4);
      str += lonstr;
      str += ",";
      str += latstr;
      //      }
      miutil::miString synopStr = str;
      miutil::miString enkelStr = str;
      double aa = datalist[i].corr(1);
      for(int j=0; j<parIndex.size();j++){
	double corr = datalist[i].corr(parIndex[j]);
	if ( parModel[j] ) {
	  for ( int k = 0; k < modeldatalist.size(); k++ ) {
	    if ( modeldatalist[k].stnr == datalist[i].stnr() &&
		 modeldatalist[k].otime == datalist[i].otime() ) {
	      corr = modeldatalist[k].orig[parIndex[j]];
	      break;
	    }
	  }
	}
	if ( parDiff[j] ) {
	  for ( int k = 0; k < modeldatalist.size(); k++ ) {
	    if ( modeldatalist[k].stnr == datalist[i].stnr() &&
		 modeldatalist[k].otime == datalist[i].otime() ) {
	      corr = corr - modeldatalist[k].orig[parIndex[j]];
	      break;
	    }
	  }
	}
	if ( parProp[j] ) {
	  for ( int k = 0; k < modeldatalist.size(); k++ ) {
	    if ( modeldatalist[k].stnr == datalist[i].stnr() &&
		 modeldatalist[k].otime == datalist[i].otime() ) {
	      if ( abs(modeldatalist[k].orig[parIndex[j]]) > 0.00001 )
		corr = corr/modeldatalist[k].orig[parIndex[j]];
	      else
		corr = -32767.0;
	      break;
	    }
	  }
	}
	if(corr<-999){
	  if(parSynop[j]) {
	    enkelStr += ",X";
	    synopStr += ",-32767";
	  }
	} else {
	  corr = dianaValue(parIndex[j], parModel[j], corr, aa);
	  int flag = datalist[i].flag(parIndex[j]);
	  int shFl1 = flag/10000;
	  int shFl2 = flag%10000/1000;
	  int shFl3 = flag%1000/100;
	  int shFl4 = flag%100/10;
	  int shFl5 = flag%10;
	  int maxFlag = shFl1 >shFl2 ? shFl1 : shFl2;
	  maxFlag = shFl3 > maxFlag ? shFl3 : maxFlag;
	  maxFlag = shFl4 > maxFlag ? shFl4 : maxFlag;
       	  miutil::miString flagstr(flag,5);
       	  miutil::miString colorstr;
	  if ( maxFlag == 0 )
       	    colorstr = ";0:0:0";
	  //	  else if ( maxFlag == 1 )
	  if ( maxFlag == 1 || (shFl5 != 0 && shFl5 != 9))
	    colorstr = ";0:255:0";
	  else if ( maxFlag >= 2 && maxFlag <= 5 )
	    colorstr = ";255:175:0";
	  else if ( maxFlag >= 6 )
	    colorstr = ";255:0:0";
	 
	  //	  miutil::miString flagstr(flag,5);
	  miutil::miString valstr = (miutil::miString)corr;
	  miutil::miString synvalstr = (miutil::miString)corr;
	  //       	  s += ":";
	  //	  if ( firstObs )
	    synvalstr += ";";
	    //	  else
	    //	    synvalstr += ":";
	  synvalstr +=flagstr;
	  if(parSynop[j]){
	    enkelStr += ",";
	    enkelStr += valstr;
	    //	    cerr << enkelStr << endl;
	    synopStr += ",";
	    synopStr += synvalstr;
	    //	    if ( firstObs) 
	      synopStr += colorstr;
	  }
	}
      }
      synopData.push_back(synopStr);
      enkelData.push_back(enkelStr);
    }
  }
  //send letters
  if(synopData.size()){
    firstObs = false;
    pLetter.description = synopDescription;
    pLetter.common = time.isoTime() + ",synop";
    pLetter.data = synopData;
    cerr << "HQC sender melding" << endl;
    cerr <<"HQC: meldingen inneholder:"<< pLetter.content() <<endl;
    pluginB->sendMessage(pLetter);
  }
  /*    
  if(enkelData.size()){
    pLetter.description = enkelDescription;
    pLetter.common = time.isoTime() + ",list";
    //    pLetter.common = time.isoTime() + ",enkel";
    pLetter.data = enkelData;
    
    //    cerr <<"ENKEL"<<endl;
    //    cerr <<"HQC: command:"<<pLetter.command<<endl;
    //    cerr <<"HQC: commondesc:"<<pLetter.commondesc<<endl;
    //    cerr <<"HQC: common:"<<pLetter.common<<endl;
    //    cerr <<"HQC: desc:"<<pLetter.description<<endl;
    
    //    for(int i=0;i<pLetter.data.size();i++)
    //      cerr <<"HQC: data:"<<pLetter.data[i]<<endl;
    cerr << "HQC sender melding" << endl;
  cerr <<"HQC: meldingen inneholder:"<< pLetter.content() <<endl;
    pluginB->sendMessage(pLetter);
  }
  */
  //  if ( firstObs ) {
  if( !synopData.size() ) {
    miMessage okLetter;
    okLetter.command = "menuok";
    okLetter.from = hqcFrom;
    okLetter.to = hqcTo;
    cerr << "HQC sender melding" << endl;
    cerr <<"HQC: meldingen inneholder:"<< okLetter.content() <<endl;
    pluginB->sendMessage(okLetter);
  }
  
}



void HqcMainWindow::sendSelectedParam(miutil::miString param)
{
       
  param = dianaName(param);
  if(!param.exists()) return;

  miMessage pLetter;
  pLetter.command = qmstrings::select_HQC_param;
  pLetter.commondesc = "param";
  pLetter.common = param;
  cerr << "HQC sender melding" << endl;
  cerr <<"HQC: meldingen inneholder:"<< pLetter.content() <<endl;
  pluginB->sendMessage(pLetter);


}


void HqcMainWindow::updateParams(int station, 
				 miutil::miTime time,
				 miutil::miString param, 
				 miutil::miString value, 
				 miutil::miString flag)
{
  //Update datalist
  int i=0;
  int n= datalist.size();
  while( i<n && ( datalist[i].stnr() !=station || datalist[i].otime() != time)) i++;
  if(i==n) return; //station and time not found
	 
  int parameterIndex=-1;
  std::list<kvalobs::kvParam>::const_iterator it=plist.begin();
	 
  for(;it!=plist.end(); it++){
    if ( it->name() == param ){
	parameterIndex = it->paramID();
	break;
    }
  }
  if(parameterIndex == -1) return; // parameter not found

  double dValue = atof(value.cStr());
  double cdValue = dianaValue(parameterIndex, false, atof(value.cStr()),datalist[i].corr(1));
  int    iflag  = atoi(flag.cStr());
  datalist[i].set_corr(parameterIndex, dValue);
  datalist[i].set_flag(parameterIndex, iflag);
  miutil::miString value_flag = miutil::miString(cdValue) + ":" + flag;

  //update timeseries
  TimeseriesOK();

  //update diana
  miMessage pLetter;
  pLetter.command = qmstrings::update_HQC_params;
  pLetter.commondesc = "time,plottype";
  pLetter.common = time.isoTime() + ",enkel";
  pLetter.description = "id," + param;
  miutil::miString data(station);
  data += ",";
  data += value_flag;
  pLetter.data.push_back(data);
  cerr << "HQC sender melding" << endl;
  cerr <<"HQC: meldingen inneholder:"<< pLetter.content() <<endl;
  pluginB->sendMessage(pLetter);

  pLetter.common = time.isoTime() + ",synop";
  miutil::miString dianaParam = dianaName(param);
  if(dianaParam.exists()){
    pLetter.description = "id," + dianaParam;
    cerr << "HQC sender melding" << endl;
    cerr <<"HQC: meldingen inneholder:"<< pLetter.content() <<endl;
    pluginB->sendMessage(pLetter);
  }
}

// Help function to translate from kvalobs parameter value to diana parameter value 
double HqcMainWindow::dianaValue(int parNo, bool isModel, double qVal, double aa) {
  double dVal;
  if ( parNo == 273 ) {
    if ( qVal <= 5000 )
      dVal = int(qVal)/100;
    else if ( qVal <= 30000 )
      dVal = int(qVal)/1000 + 50;
    else if ( qVal < 75000 )
      dVal = int(qVal)/5000 + 74;
    else
      dVal = 89;
    return dVal;
  }
  else if ( parNo == 55 ) {
    if ( qVal < 50 )
      dVal = 0;
    else if ( qVal < 100 )
      dVal = 1;
    else if ( qVal < 200 )
      dVal = 2;
    else if ( qVal < 300 )
      dVal = 3;
    else if ( qVal < 600 )
      dVal = 4;
    else if ( qVal < 1000 )
      dVal = 5;
    else if ( qVal < 1500 )
      dVal = 6;
    else if ( qVal < 2000 )
      dVal = 7;
    else if ( qVal < 2500 )
      dVal = 8;
    else
      dVal = 9;
    return dVal;
 }
  else if ( parNo == 177 ) {
    if ( aa >= 5.0 && !isModel ) {
      dVal = -qVal;
    }
    else {
      dVal = qVal;
    }
    return dVal;
  }
  //  else if ( parNo > 80 && parNo < 95 ) {
  //    dVal = 1.94384*qVal;
  //    return dVal;
  //  }
  else
    return qVal;
}


// Help function to translate from qualobs parameter names to diana parameter names 
miutil::miString HqcMainWindow::dianaName(miutil::miString lbl) {
  NameMap::iterator dnit;
  for ( dnit = dnMap.begin(); dnit != dnMap.end(); dnit++ ) {
    if ( lbl == dnit.data() ) {
      return dnit.key();
    }
  }
  return "";
}

miutil::miString HqcMainWindow::hqcType(int typeId, int env) {
  // Generates string to send to Diana
  miutil::miString hqct = "none";
  if ( env == 8 ) {
    if ( typeId == 3 || typeId == 330 || typeId == 342 )
      hqct = "AA";
    else if ( typeId == 1 || typeId == 6 || typeId == 312 || typeId == 412 )
      hqct = "VS";
    else if ( typeId == 306 )
      hqct = "VM";
  }
  else if ( env == 2 ) {
    if ( typeId == 3 )
      hqct = "AL";
  }
  else if ( env == 12 ) {
    if ( typeId == 3 )
      hqct = "AV";
  }
  else if ( env == 3 ) {
    if ( typeId == 3 || typeId == 410 )
      hqct = "AP";
    else if ( typeId == 412 )
      hqct = "VK";
  }
  else if ( env == 1 ) {
    if ( typeId == 310 || typeId == 311 || typeId == 410 || typeId == 1 )
      hqct = "AF";
    else if ( typeId == 2 )
      hqct = "FM";
    else if ( typeId == 1 || typeId == 6 || typeId == 312 || typeId == 412 )
      hqct = "VS";
  }
  else if ( env == 5 ) {
    if ( typeId == 6 || typeId == 430 )
      hqct = "AE";
    else if ( typeId == 11 )
      hqct = "MP";
  }
  else if ( env == 7 ) {
    if ( typeId == 11 )
      hqct = "MV";
  }
  else if ( env == 4 ) {
    if ( typeId == 11 )
      hqct = "MM";
  }
  else if ( env == 6 ) {
    if ( typeId == 11 )
      hqct = "MS";
  }
  else if ( env == 9 ) {
    if ( typeId == 302 || typeId == 303 )
      hqct = "NS";
    else if ( typeId == 402 )
      hqct = "ND";
    else if ( typeId == 4 || typeId == 405 )
      hqct = "P ";
  }
  else if ( env == 10 ) {
    if ( typeId == 402 )
      hqct = "NO";
  }
  else if ( env == 11 ) {
    if ( typeId == 309 )
      hqct = "VT";
  }
  return hqct;
} 

void HqcMainWindow::updateSaveFunction( QWidget *w )
{
  MDITabWindow *win = dynamic_cast<MDITabWindow*>(w);
  bool enabled = ( win and win->erl );
  file->setItemEnabled( fileSaveMenuItem, enabled );
  cerr << "Save " << (enabled? "en":"dis") << "abled\n";
}

bool HqcMainWindow::isAlreadyStored(miutil::miTime otime, int stnr) {
  for ( int i = 0; i < datalist.size(); i++) {
    if ( datalist[i].otime() == otime && datalist[i].stnr() == stnr )
      return true;
  }
  return false;
}
 
int HqcMainWindow::findTypeId(int typ, int pos, int par, miutil::miTime oTime)
{
  int tpId;
  tpId = typ;
  for(CIObsPgmList obit=obsPgmList.end();obit!=obsPgmList.begin(); obit--){
    if ( obit->stationID() == pos && obit->paramID() == par && obit->fromtime() < oTime) {
      tpId = obit->typeID();
      break;
    }
  }
  if ( abs(tpId) > 503 ) {
    switch (par) {
    case 106:
      for(CIObsPgmList obit=obsPgmList.end();obit!=obsPgmList.begin(); obit--){
	if ( obit->stationID() == pos && obit->paramID() == 105 && obit->fromtime() < oTime) {
	  tpId = -obit->typeID();
	  break;
	}
      }
      break;
    case 109:
      for(CIObsPgmList obit=obsPgmList.end();obit!=obsPgmList.begin(); obit--){
	if ( obit->stationID() == pos && (obit->paramID() == 104 || obit->paramID() == 105 || obit->paramID() == 106) && obit->fromtime() < oTime) {
	  tpId = -obit->typeID();
	  break;
	}
      }
      break;
    case 110:
      for(CIObsPgmList obit=obsPgmList.end();obit!=obsPgmList.begin(); obit--){
	if ( obit->stationID() == pos && (obit->paramID() == 104 || obit->paramID() == 105 || obit->paramID() == 106 || obit->paramID() == 109) && obit->fromtime() < oTime) {
	  tpId = -obit->typeID();
	  break;
	}
      }
      break;
    case 214:
      for(CIObsPgmList obit=obsPgmList.end();obit!=obsPgmList.begin(); obit--){
	if ( obit->stationID() == pos && obit->paramID() == 213 && obit->fromtime() < oTime) {
	  tpId = -obit->typeID();
	  break;
	}
      }
      break;
    case 216:
      for(CIObsPgmList obit=obsPgmList.end();obit!=obsPgmList.begin(); obit--){
	if ( obit->stationID() == pos && obit->paramID() == 215 && obit->fromtime() < oTime) {
	  tpId = -obit->typeID();
	  break;
	}
      }
      break;
    case 224:
      for(CIObsPgmList obit=obsPgmList.end();obit!=obsPgmList.begin(); obit--){
	if ( obit->stationID() == pos && obit->paramID() == 223 && obit->fromtime() < oTime) {
	  tpId = -obit->typeID();
	  break;
	}
      }
      break;
    default:
      tpId = -32767;;
    }
  }
  return tpId;
}

void 
HqcMainWindow::
makeTextDataList( KvObsDataList& textDataList )
{
  //  cout << "textDataList.size = " << textDataList.size() << endl;
  for(IKvObsDataList it=textDataList.begin(); it!=textDataList.end(); it++ ) {
    KvObsData::kvTextDataList::iterator dit=it->textDataList().begin();
    while( dit != it->textDataList().end() ) {
      TxtDat txtd;
      txtd.stationId = dit->stationID();
      txtd.obstime   = dit->obstime(); 
      txtd.original  = dit->original(); 
      txtd.paramId   = dit->paramID(); 
      txtd.tbtime    = dit->tbtime(); 
      txtd.typeId    = dit->typeID();
      txtList.push_back(txtd);
      dit++;
    }
  }
}

void 
HqcMainWindow::
makeObsDataList( KvObsDataList& dataList )
{
  model::KvalobsData tdl;
  bool tdlUpd[NOPARAM];
  std::fill(tdlUpd, tdlUpd + NOPARAM, false);

  miutil::miTime protime("1800-01-01 00:00:00");
  int prtypeId = -1;
  int prstnr = 0;
  int aggPar = 0;
  int aggTyp = 0;
  int aggStat = 0;
  miutil::miTime aggTime("1800-01-01 00:00:00") ;

  for(IKvObsDataList it=dataList.begin(); it!=dataList.end(); it++ ) {
    KvObsData::kvDataList::iterator dit=it->dataList().begin();
    //    IDataList dit = it->dataList().begin();
    int ditSize = it->dataList().size();
    int stnr = dit->stationID();
    int prParam = -1;
    int prSensor = -1;
    int ditNo = 0;
    while( dit != it->dataList().end() ) {
      int astnr = dit->stationID();
      bool correctLevel = (dit->level() == HqcMainWindow::sLevel );
      bool correctTypeId;
      if ( lstdlg->allTypes->isChecked() && dit->sensor() - '0' == 0)
	correctTypeId = true;
      else
	correctTypeId = HqcMainWindow::typeIdFilter(stnr, dit->typeID(), dit->sensor() - '0', dit->obstime(), dit->paramID() );
      //      if ( dit->typeID() < 0 && dit->typeID() != -342 ) {
      if ( dit->typeID() < 0 ) {
	aggPar = dit->paramID();
	aggTyp = dit->typeID();
	aggTime = dit->obstime();
	aggStat = dit->stationID();
      }

      int stnr = dit->stationID();
      miutil::miTime otime = (dit->obstime());
      miutil::miTime tbtime = (dit->tbtime());
      int hour = otime.hour();
      int typeId = dit->typeID();
      int sensor = dit->sensor();
      if ( otime == protime && stnr == prstnr && dit->paramID() == prParam && typeId == prtypeId && sensor == prSensor 
	   && lstdlg->priTypes->isChecked() ) {
	protime = otime;
	prstnr = stnr;
	prtypeId = typeId;
	prSensor = sensor;
	prParam = -1;
	dit++;
	ditNo++;
	continue;
      }
      tdl.set_otime(otime);
      tdl.set_tbtime(tbtime);
      tdl.set_stnr(stnr);
      bool isaggreg = ( stnr == aggStat && otime == aggTime && typeId == abs(aggTyp) && aggPar == dit->paramID());
      if ( correctTypeId && correctLevel && !isaggreg && !tdlUpd[dit->paramID()] ) {
	tdl.set_typeId(dit->paramID(), typeId);
	tdl.set_showTypeId(typeId);
	tdl.set_orig(dit->paramID(), dit->original());
	tdl.set_flag(dit->paramID(), getShowFlag(dit->controlinfo()));
	tdl.set_corr(dit->paramID(), dit->corrected());
	tdl.set_sensor(dit->paramID(), dit->sensor());
	tdl.set_level(dit->paramID(), dit->level());
	tdl.set_controlinfo(dit->paramID(), dit->controlinfo().flagstring());
      	tdl.set_useinfo(dit->paramID(), dit->useinfo().flagstring());
	tdl.set_cfailed(dit->paramID(), dit->cfailed());
	tdlUpd[dit->paramID()] = true;
      }
      protime = otime;
      prstnr = stnr;
      prtypeId = typeId;
      prSensor = sensor;
      prParam = dit->paramID();
      QString name;
      double lat, lon, hoh;
      int env;
      int snr;
      findStationInfo(stnr, name, lat, lon, hoh, snr, env);
      tdl.set_name(name);
      tdl.set_snr(snr);
      int prid = dit->paramID();
      bool correctHqcType = hqcTypeFilter(tdl.showTypeId(), env, stnr); //  !!!
      ++dit;
      ++ditNo;
      otime = dit->obstime();
      stnr = dit->stationID();
      typeId = dit->typeID();
      bool errFl = false;
      if ( (!correctHqcType || !correctLevel || !correctTypeId) && ditNo < ditSize - 1 ) {
	continue;
      }
      else if ( ditNo == ditSize - 1 ) goto pushback;
      for ( int ip = 0; ip < NOPARAM; ip++) {
	int shFl  = tdl.flag(ip);
	int shFl1 = shFl/10000;
	int shFl2 = shFl%10000/1000;
	int shFl3 = shFl%1000/100;
	int shFl4 = shFl%100/10;
	if ( shFl1 > 1 || shFl2 > 1 || shFl3 > 1 || shFl4 > 1 )
	  errFl = true;
      }
      if ( !errFl && (lity == erLi || lity == erSa || lity == erLo) ) {
	continue;
      }
    pushback:
      if ( (timeFilter(hour) && !isAlreadyStored(protime, prstnr) &&
	    ((otime != protime || ( otime == protime && stnr != prstnr)))) || (lstdlg->allTypes->isChecked() && typeId != prtypeId) ) {
	datalist.push_back(tdl);
	tdl = model::KvalobsData();
	std::fill(tdlUpd, tdlUpd + NOPARAM, false);
      }
      else if ( !timeFilter(hour) ) {
        tdl = model::KvalobsData();
        std::fill(tdlUpd, tdlUpd + NOPARAM, false);
      }
    }
  }
}
