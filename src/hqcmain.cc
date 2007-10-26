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
#include <iomanip>
#include <qsgistyle.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qplatinumstyle.h>
#include <qwindowsstyle.h>
#include <qcdestyle.h>
#include <qmotifplusstyle.h>
#include <qcommonstyle.h>
#include <qvalidator.h>
#include <qmetaobject.h>
#include <TSPlot.h>
#include <glTextX.h>
#include <kvData.h>
#include "identifyUser.h"
#include "BusyIndicator.h"
#include "RRDialog.h"
#include "weatherdialog.h"
#include <deque>
#include <stdexcept>
#include <complex>

using namespace std;

const miutil::miString DATASET_STATIONS = "TESTPOSISJONER";

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
  : QMainWindow( 0, "HQC", WDestructiveClose ), usesocket(true)
  , reinserter( NULL )
{
  // --- CHECK USER IDENTITY ----------------------------------------

  //  reinserter = Authentication::identifyUser( dynamic_cast<KvApp *>(qApp),
  reinserter = Authentication::identifyUser(  KvApp::kvApp,
					     "ldap.oslo.dnmi.no", userName);
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
    cout << "Hei  " << userName << ", du er registrert som godkjent operatør" << endl;
  }
  //-----------------------------------------------------------------  

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
#ifdef linux
  //  qApp->setStyle(new QPlatinumStyle);
  //  qApp->setStyle(new QWindowsStyle);
#else
  qApp->setStyle(new QSGIStyle);
#endif
  listExist = FALSE;
  menuBar()->setFont(QFont("system", 12));
  
  file = new QPopupMenu( this );
  menuBar()->insertItem( "&Fil", file );
  file->insertSeparator();

  fileSaveMenuItem = file->insertItem( "Lagre", this, SIGNAL( saveData() ), CTRL+Key_S );
  file->setItemEnabled( fileSaveMenuItem, false );
  //KTEST
  filePrintMenuItem = file->insertItem( "Skriv ut", this, SIGNAL( printErrorList() ), CTRL+Key_P );
  file->setItemEnabled( filePrintMenuItem, false );

  file->insertItem( "&Lukk",    this, SLOT(closeWindow()), CTRL+Key_W );
  file->insertItem( "&Avslutt", qApp, SLOT( closeAllWindows() ), CTRL+Key_Q );
  
  choice = new QPopupMenu( this );
  choice->setCheckable(TRUE);
  menuBar()->insertItem( "&Valg", choice );
  choice->insertSeparator();
  flID = choice->insertItem( "Vis flagg",                 this, SLOT(showFlags()));
  orID = choice->insertItem( "Vis original",              this, SLOT(showOrigs()));
  moID = choice->insertItem( "Vis modeldata",             this, SLOT(showMod()));
  stID = choice->insertItem( "Vis stasjonsnavn",          this, SLOT(showStat()));
  poID = choice->insertItem( "Vis lengde, bredde, høyde", this, SLOT(showPos()));
  //  tyID = choice->insertItem( "Vis alle typer",            this, SLOT(showTyp()));
  isShFl = TRUE;
  isShOr = TRUE;
  isShMo = TRUE;
  isShSt = TRUE;

  choice->setItemChecked(flID, isShFl);
  choice->setItemChecked(orID, isShOr);
  choice->setItemChecked(moID, isShMo);
  choice->setItemChecked(stID, isShSt);

  
  showmenu = new QPopupMenu( this );
  menuBar()->insertItem( "&Listetype", showmenu);
  showmenu->insertItem( "Data&liste og Feilliste    ", this, SLOT(allListMenu()),ALT+Key_L );
  showmenu->insertItem( "&Feilliste    ", this, SLOT(errListMenu()),ALT+Key_F );
  showmenu->insertItem( "F&eillog    ",   this, SLOT(errLogMenu()),ALT+Key_E );
  showmenu->insertItem( "&Dataliste    ", this, SLOT(dataListMenu()),ALT+Key_D );
  showmenu->insertItem( "&Feilliste salen", this, SLOT(errLisaMenu()),ALT+Key_S );
  showmenu->insertSeparator();
  showmenu->insertItem( "&Tidsserie    ", this, SLOT(timeseriesMenu()),ALT+Key_T );
  showmenu->insertItem( "&Nedbør", this, SLOT( showWatchRR() ), CTRL+Key_R );
  showmenu->insertSeparator();
  showmenu->insertItem( "&Vær", this, SLOT( showWeather() ), CTRL+Key_V );
  showmenu->insertSeparator();
  
  weathermenu = new QPopupMenu( this );
  menuBar()->insertItem( "&Værelement", weathermenu);
  taID = weathermenu->insertItem( "&Temperatur og fuktighet", this, SLOT(temperature()) );
  prID = weathermenu->insertItem( "&Nedbør og snøforhold",    this, SLOT(precipitation()) );
  apID = weathermenu->insertItem( "&Lufttrykk og vind",       this, SLOT(airPress()) );
  clID = weathermenu->insertItem( "&Visuelle parametere",     this, SLOT(visuals()) );
  seID = weathermenu->insertItem( "&Maritime parametere",     this, SLOT(sea()) );
  syID = weathermenu->insertItem( "&Synop",                   this, SLOT(synop()) );
  klID = weathermenu->insertItem( "&For klimastatistikk",     this, SLOT(climateStatistics()) );
  piID = weathermenu->insertItem( "&Prioriterte parametere",  this, SLOT(priority()) );
  wiID = weathermenu->insertItem( "&Vind",                    this, SLOT(wind()) );
  plID = weathermenu->insertItem( "&Pluviometerkontroll",     this, SLOT(plu()) );
  alID = weathermenu->insertItem( "&Alt",                     this, SLOT(all()) );

  clockmenu = new QPopupMenu( this );
  menuBar()->insertItem( "&Tidspunkter", this, SLOT(clk()));
  menuBar()->insertItem( "&Dianavisning", this, SLOT(dsh()));
  menuBar()->insertItem( "&Kro", this, SLOT(startKro()));
  QPopupMenu * help = new QPopupMenu( this );
  menuBar()->insertItem( "&Hjelp", help );
  help->insertItem( "&Hjelp", this, SLOT(help()), Key_F1);
  help->insertSeparator();
  help->insertItem( "&Om Hqc", this, SLOT(about()));
  help->insertSeparator();
  help->insertItem( "Om &Qt", this, SLOT(aboutQt()));
  
  // --- MAIN WINDOW -----------------------------------------
  QVBox* vb = new QVBox( this );
  vb->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
  //    vb->setGeometry(10,100,1000,1000);
  ws = new QWorkspace( vb );
  ws->setScrollBarsEnabled( TRUE );
  ws->setBackgroundColor( white );
  setCentralWidget( vb );

  connect( ws, SIGNAL( windowActivated(QWidget*) ),
	   this, SLOT( updateSaveFunction(QWidget*) ) );

  
  // --- TOOL BAR -----------------------------------------
  QString path = QString(getenv("HQCDIR"));
  QPixmap icon_listdlg(path + "/table.png");
  QPixmap icon_ts(path + "/kmplot.png");
  QToolBar * hqcTools = new QToolBar( this, "hqc" );
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
  
  if(usesocket){
    pluginB = new ClientButton("hqc",
			       "/metno/local/bin/coserver",
			       statusBar());
    pluginB->useLabel(true);
    pluginB->openConnection();
    
    connect(pluginB, SIGNAL(receivedLetter(miMessage&)),
	    SLOT(processLetter(miMessage&)));
    statusBar()->addWidget(pluginB,0,true);
  }
  statusBar()->message( "Ready", 2000 );
  
  // --- READ STATION INFO ----------------------------------------

  readFromStation();
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
    //    if ( tpind == -1 ) {
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
  // --- DEFINE DIALOGS --------------------------------------------
  lstdlg = new ListDialog(this);
  clkdlg = new ClockDialog(this);
  pardlg = new ParameterDialog(this);
  dshdlg = new DianaShowDialog(this);
  // --- READ PARAMETER INFO ---------------------------------------
  
  readFromParam();
 
  // --- START -----------------------------------------------------
  pardlg->hide();
 
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
  glText* gltext= new glTextX();
  gltext->testDefineFonts();
  //  FM.addFontCollection(gltext, XFONTSET);
  //  FM.setFontColl(XFONTSET);
}

void HqcMainWindow::setKvBaseUpdated(bool isUpdated) {
  kvBaseIsUpdated = isUpdated;
}

void HqcMainWindow::showFlags() {
  isShFl = !isShFl;
  choice->setItemChecked(flID, isShFl);
  //  if ( listExist )
  //    emit toggleShow();
}

void HqcMainWindow::showOrigs() {
  isShOr = !isShOr;
  choice->setItemChecked(orID, isShOr);
  //  if ( listExist )
  //    emit toggleShow();
}

void HqcMainWindow::showMod() {
  isShMo = !isShMo;
  choice->setItemChecked(moID, isShMo);
  //  if ( listExist )
  //    emit toggleShow();
}

void HqcMainWindow::showStat() {
  isShSt = !isShSt;
  choice->setItemChecked(stID, isShSt);
  //  if ( listExist )
  //    emit toggleShow();
}

void HqcMainWindow::showPos() {
  isShPo = !isShPo;
  choice->setItemChecked(poID, isShPo);
  //  if ( listExist )
  //    emit toggleShow();
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
  //  if ( listExist ){
  //    emit toggleWeather();
  //  }
  sendObservations(remstime,false);
}

void HqcMainWindow::temperature() {
  wElement = "Temperatur";
  lity = daLi;
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
//  if ( listExist ){
//    emit toggleWeather();
//  }
  sendObservations(remstime,false);
}

void HqcMainWindow::precipitation() {
  wElement = "Nedbør";
  lity = daLi;
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
//  if ( listExist )
//    emit toggleWeather();
  sendObservations(remstime,false);
  insertParametersInListBox(NOPARAMPREC, precOrder);
  pardlg->showAll();
}

void HqcMainWindow::visuals() {
  wElement = "Visuell";
  lity = daLi;
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
  //  if ( listExist )
  //    emit toggleWeather();
  sendObservations(remstime,false);
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
 //  if ( listExist )
  //    emit toggleWeather();
  sendObservations(remstime,false);
  insertParametersInListBox(NOPARAMWAVE, waveOrder);
  pardlg->showAll();
}

void HqcMainWindow::synop() {
  wElement = "Synop";
  lity = daLi;
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
  //  if ( listExist )
  //    emit toggleWeather();
  sendObservations(remstime,false);
  insertParametersInListBox(NOPARAMSYNOP, synopOrder);
  pardlg->showAll();
}

void HqcMainWindow::climateStatistics() {
  wElement = "Klimastatistikk";
  lity = daLi;
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
  //  if ( listExist )
  //    emit toggleWeather();
  sendObservations(remstime,false);
  insertParametersInListBox(NOPARAMKLSTAT, klstatOrder);
  pardlg->showAll();
}

void HqcMainWindow::priority() {
  wElement = "Prioriterte parametere";
  lity = daLi;
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
  //  if ( listExist )
  //    emit toggleWeather();
  sendObservations(remstime,false);
  insertParametersInListBox(NOPARAMPRIORITY, priorityOrder);
  pardlg->showAll();
}

void HqcMainWindow::wind() {
  wElement = "Vind";
  lity = daLi;
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
  //  if ( listExist )
  //    emit toggleWeather();
  sendObservations(remstime,false);
  insertParametersInListBox(NOPARAMWIND, windOrder);
  pardlg->showAll();
}

void HqcMainWindow::plu() {
  wElement = "Pluviometerkontroll";
  lity = daLi;
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
  //  if ( listExist )
  //    emit toggleWeather();
  sendObservations(remstime,false);
  insertParametersInListBox(NOPARAMPLU, pluOrder);
  pardlg->showAll();
}

void HqcMainWindow::all() {
  wElement = "Alt";
  lity = daLi;
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
  //  if ( listExist )
  //    emit toggleWeather();
  sendObservations(remstime,false);
  insertParametersInListBox(NOPARAMALL, order);
  pardlg->showAll();
}

void HqcMainWindow::paintEvent(QPaintEvent*) { 
  QString path = QString(getenv("HQCDIR"));
  QPixmap image1(path + "/hqc.png");
  logo = new QPainter;
  logo->begin(ws);
  logo->scale(1.2,1.2);
  logo->drawPixmap(33,39,image1);
  logo->end();
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
    //   QMessageBox::critical(this, 
    QMessageBox::warning(this, 
			 "Stasjonsvalg", 
			 "Ingen stasjoner er valgt!\n"
			 "Minst en stasjon må velges", 
			  QMessageBox::Ok, 
			  QMessageBox::NoButton);
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
			  QMessageBox::NoButton);
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
  if ( lity != erLi && lity != erSa )
    sendAnalysisMessage();
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
  if ( (lity != erLi && lity != erSa) || 
       ( lity == erLi && remLity == erLi) || 
       ( lity == erSa && remLity == erSa) )
    closeWindow();
  if ( lity == erLi || lity == erSa || lity == daLi) {
    /*
    metty = tabHead;
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
	   userName);
    */
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
  cerr << "TimeseriesOK started" << endl;
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
	if ( datalist[i].stnr == stationIndex[ip] &&
	     datalist[i].otime >= stime &&
             datalist[i].otime <= etime &&
	     datalist[i].otime.min() == 0 ) {
	  tseries.add(TimeSeriesData::Data(datalist[i].otime,
					   datalist[i].corr[parameterIndex[ip]]));
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
					  datalist[i].corr[parameterIndex[ip]] 
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
  WatchRR::RRDialog * rrd = WatchRR::RRDialog::getRRDialog( data, this );
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
  Weather::WeatherDialog * wtd = Weather::WeatherDialog::getWeatherDialog( data, this );
  if ( wtd ) {
    wtd->setReinserter( reinserter );
    wtd->show();
  }
}

void HqcMainWindow::listMenu() {
  if ( lstdlg->isVisible() ) {
    lstdlg->hideAll();
  } else {
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
  //  system("konqueror modon/cgi-bin/tiltak/start.pl &");
  //  system("mozilla modon/cgi-bin/tiltak/start.pl &");
  system("mozilla kro/cgi-bin/start.pl &");
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
  stnr = datalist[index].stnr;
  obstime = datalist[index].otime;
  showTypeId = datalist[index].showTypeId;
  typeIdChanged = datalist[index].typeIdChanged;
  int hour = obstime.hour();
  for ( int i = 0; i < NOPARAM; i++ ) {
    typeId[i] = datalist[index].typeId[i];
    orig[i] = datalist[index].orig[i];
    flag[i] = datalist[index].flag[i];
    corr[i] = datalist[index].corr[i];
    controlinfo[i] = datalist[index].controlinfo[i];
    useinfo[i] = datalist[index].useinfo[i];
    cfailed[i] = datalist[index].cfailed[i];
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
  int code;
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
  int code;
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
  int code;
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
  int code;
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
  int code;
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
  int code;
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
  int code;
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
  int code;
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

bool HqcMainWindow::hqcTypeFilter(int& typeId, int environment, int stnr) {
  if ( lstdlg->webReg->isChecked() || lstdlg->priReg->isChecked() ) return TRUE;
  //  if ( typeId == -1 ) return FALSE;
  int atypeId = typeId < 0 ? -typeId : typeId;
  if (  lstdlg->allType->isChecked() ) return TRUE;
  if ( environment == 1 && atypeId == 311 && lstdlg->afType->isChecked() ) return TRUE;
  if ( (environment == 8 && (atypeId == 3 || atypeId == 311 || atypeId == 412 || atypeId == 501)) || (atypeId == 330 || atypeId == 342) && lstdlg->aaType->isChecked() ) return TRUE;
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

bool HqcMainWindow::typeIdFilter(int stnr, int typeId, miutil::miTime otime, bool allTypes) {
  if ( allTypes ) return true;
  bool tpf = false;
  for ( vector<currentType>::iterator it = currentTypeList.begin(); it != currentTypeList.end(); it++) {
    if ( stnr == (*it).stnr && abs(typeId) == (*it).cTypeId && (*it).status == "D" && otime.date() >= (*it).fDate && otime.date() <= (*it).tDate )
      tpf = true;
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

  //  vector<datl> tempDatalist;
  bool result;
  dlist.erase(dlist.begin(),dlist.end());
  datalist.clear();
  for ( int ip = 0; ip < NOPARAM; ip++) {
    tdl.orig[ip]   = -32767.0;
    tdl.flag[ip]   = 0;
    tdl.corr[ip]   = -32767.0;
    tdl.sensor[ip] = 0;
    tdl.level[ip]  = 0;
    tdl.typeId[ip] = -32767;
    tdl.controlinfo[ip] = "";
    tdl.cfailed[ip] = "";
  }
  miutil::miTime protime("1900-01-01 00:00:00");
  int prtypeId = -1;
  int prstnr = 0;
  WhichDataHelper whichData;
  for ( int i = 0; i < stList.size(); i++ ) {
    whichData.addStation(stList[i], stime, etime);
    readFromTypeIdFile(stList[i]);
  }
  KvObsDataList ldlist;

  if(!KvApp::kvApp->getKvData(ldlist, whichData))
    cerr << "Can't connect to data table!" << endl;
  //  bool aggreg = false;
  int aggPar = 0;
  int aggTyp = 0;
  int aggStat = 0;
  miutil::miTime aggTime;
  for(IKvObsDataList it=ldlist.begin(); it!=ldlist.end(); it++ ) {
    IDataList dit = it->dataList().begin();
    int stnr = dit->stationID();
    int prParam = -1;
    bool allTypes = true;
    while( dit != it->dataList().end() ) {
      bool correctLevel = (dit->level() == sLevel );
      bool correctSensor = (dit->sensor() - '0' == sSensor );
      bool correctTypeId = typeIdFilter(stnr, dit->typeID(), dit->obstime(), allTypes );

      //      cerr << "Knut tester Myken : " << stnr << " " << dit->paramID() << " " << dit->typeID() << " " << dit->sensor() << " " << dit->obstime() << " " << correctTypeId << endl;

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
            
      if ( (otime == protime && stnr == prstnr && dit->paramID() == prParam) ) {
	protime = otime;
	prstnr = stnr;
	prtypeId = typeId;
	prParam = -1;
	dit++;
	//	cerr << "Knut tester Myken igjen: " << endl;
	continue;
      }
      tdl.otime = otime;
      tdl.tbtime = tbtime;
      tdl.stnr = stnr;
      bool isaggreg = ( stnr == aggStat && otime == aggTime && typeId == abs(aggTyp) && aggPar == dit->paramID());
      if ( (correctTypeId && correctLevel && correctSensor && typeId != 501 && !isaggreg) ) {
	tdl.typeId[dit->paramID()] = typeId;
	if ( typeId == -6 || typeId == -330 || typeId == -342 || typeId == 306 || typeId == 308 ) {
	  prParam = dit->paramID();
	}
	tdl.showTypeId = typeId;
	tdl.orig[dit->paramID()] = dit->original();
	tdl.flag[dit->paramID()] = getShowFlag(dit->controlinfo());
	tdl.corr[dit->paramID()] = dit->corrected();
	tdl.sensor[dit->paramID()] = dit->sensor();
	tdl.level[dit->paramID()] = dit->level();
	tdl.controlinfo[dit->paramID()] = dit->controlinfo().flagstring();
      	tdl.useinfo[dit->paramID()] = dit->useinfo().flagstring();
	tdl.cfailed[dit->paramID()] = dit->cfailed();
      }
      protime = otime;
      prstnr = stnr;
      prtypeId = typeId;
      QString name;
      double lat, lon, hoh;
      int env;
      int snr;
      findStationInfo(stnr, name, lat, lon, hoh, snr, env);
      tdl.name = name;
      tdl.snr = snr;
      int prid = dit->paramID();
      bool correctHqcType = hqcTypeFilter(tdl.showTypeId, env, stnr);
      ++dit;
      otime = dit->obstime();
      stnr = dit->stationID();
      typeId = dit->typeID();
      if ( !correctHqcType || !correctSensor ) {
	//      	++dit;
	continue;
      }
      bool errFl = false;
      for ( int ip = 0; ip < NOPARAM; ip++) {
	int shFl  = tdl.flag[ip];
	int shFl1 = shFl/10000;
	int shFl2 = shFl%10000/1000;
	int shFl3 = shFl%1000/100;
	int shFl4 = shFl%100/10;
	if ( shFl1 > 1 || shFl2 > 1 || shFl3 > 1 || shFl4 > 1 )
	  errFl = true;
      }
      if ( !errFl && (lity == erLi || lity == erSa || lity == erLo) ) {
	//	++dit;
	continue;
      }

      if ( timeFilter(hour) && 
	   ((isShTy && prtypeId != typeId) || (otime != protime || ( otime == protime && stnr != prstnr)))) {
	datalist.push_back(tdl);
	for ( int ip = 0; ip < NOPARAM; ip++) {
	  tdl.orig[ip]   = -32767.0;
	  tdl.flag[ip]   = 0;
	  tdl.corr[ip]   = -32767.0;
	  tdl.sensor[ip] = -0;
	  tdl.level[ip]  = -0;
	  tdl.typeId[ip] = -32767;
	  tdl.controlinfo[ip] = "";
	  tdl.cfailed[ip] = "";
	}
      }
      else if ( !timeFilter(hour) ) {
	for ( int ip = 0; ip < NOPARAM; ip++) {
	  tdl.orig[ip]   = -32767.0;
	  tdl.flag[ip]   = 0;
	  tdl.corr[ip]   = -32767.0;
	  tdl.sensor[ip] = -0;
	  tdl.level[ip]  = -0;
	  tdl.typeId[ip] = -32767;
	  tdl.controlinfo[ip] = "";
	  tdl.cfailed[ip] = "";
	}
      }
    }
  }
}


/*!
 Read the modeldata table in the Qualobs database
*/

void HqcMainWindow::readFromModelData(const miutil::miTime& stime, 
				      const miutil::miTime& etime) {

  bool result;
  
  mdlist.erase(mdlist.begin(),mdlist.end());
  modeldatalist.clear();  
  WhichDataHelper whichData;
  for ( int i = 0; i < stList.size(); i++ ) {
    whichData.addStation(stList[i], stime, etime);
  }

  if(!KvApp::kvApp->getKvModelData(mdlist, whichData))
    cerr << "Can't connect to modeldata table!" << endl;
  modDatl mtdl;
  for ( int ip = 0; ip < NOPARAMMODEL; ip++) {
    mtdl.orig[modelParam[ip]] = -32767.0;
  }
  miutil::miTime protime("1900-01-01 00:00:00");
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
}

/*!
 Read the station table in the Qualobs database
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
}
/*!
 Read the obs_pgm table in the Qualobs database
*/

void HqcMainWindow::readFromObsPgm() {
  if (!KvApp::kvApp->getKvObsPgm(obsPgmList, statList, FALSE))
    cerr << "Can't connect to obs_pgm table!" << endl;
}

/*!
 Read the typeid file
*/

void HqcMainWindow::readFromTypeIdFile(int stnr) {
  QString path = QString(getenv("HQCDIR"));
  if ( !path ) {
    cerr << "Intet environment" << endl;
    exit(1);
  }
  QString typeIdFile = path + "/typeids";
  QFile typeIds(typeIdFile);
  typeIds.open(IO_ReadOnly);
  QTextStream typeIdStream(&typeIds);
  while ( typeIdStream.atEnd() == 0 ) {
    QString statLine = typeIdStream.readLine();
    int stationId = statLine.left(10).toInt();
    if ( stationId == stnr ) {
      QString status = statLine.mid(11,1);
      miutil::miDate fDate;
      fDate.setDate(miutil::miString(statLine.mid(13,10).latin1()));
      miutil::miDate tDate;
      tDate.setDate(miutil::miString(statLine.mid(24,10).latin1()));
      int typeId = statLine.stripWhiteSpace().right(3).toInt();
      crT.stnr = stationId;
      crT.status = status;
      crT.fDate = fDate;
      crT.tDate = tDate;
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
  if ( !path ) {
    cerr << "Intet environment" << endl;
    exit(1);
  }
  QString stationFile = path + "/hqc_stations";
  QFile stations(stationFile);
  stations.open(IO_ReadOnly);
  QTextStream stationStream(&stations);
  int i = 0;
  int prevStnr = 0;
  while ( stationStream.atEnd() == 0 ) {
    QString statLine = stationStream.readLine();
    int stnr = statLine.left(7).toInt();
    if ( stnr == prevStnr ) continue;
    std::list<kvalobs::kvStation>::const_iterator it=slist.begin();
    bool foundStation = FALSE;
    for(;it!=slist.end(); it++){
      if ( it->stationID() == stnr ) {
	foundStation = TRUE;
	break;
      }
    }
    if ( foundStation ) {
      QString strStnr;
      QString strHoh;
      QString strEnv;
      strEnv = strEnv.setNum(it->environmentid());
      listStatName.append(it->name());
      listStatNum.append(strStnr.setNum(it->stationID()));
      listStatHoh.append(strHoh.setNum(it->height()));
      listStatType.append(strEnv);
      listStatFylke.append(statLine.mid(8,30).stripWhiteSpace());
      listStatKommune.append(statLine.mid(39,24).stripWhiteSpace());
      listStatWeb.append(statLine.mid(64,3).stripWhiteSpace());
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
  if ( !path ) {
    cerr << "Intet environment" << endl;
    exit(1);
  }
  
  QString orderFile = path + "/paramorder";
  QFile paramOrder(orderFile);
  
  paramOrder.open(IO_ReadOnly);
  QTextStream paramStream(&paramOrder);
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
    parMap[it->paramID()] = it->name();
    listParName.append(it->name());
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
  
  int height[] = {0, 28 + ws->height() / 2, (ws->height() / 2) - 28} ;
  int y = 0;
  for ( int i = int(windows.count()) - 2; i < int(windows.count()); ++i ) {
    QWidget *window = windows.at(i);
    if ( window->testWState( WState_Maximized ) ) {
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

MDITabWindow* HqcMainWindow::eTable(const miutil::miTime& stime,
				    const miutil::miTime& etime,
				    miutil::miTime& remstime,
				    miutil::miTime& remetime,
				    listType lity,
				    listType remLity,
				    mettType metty,
				    QString& wElement,
				    int* selParNo,
				    vector<datl>& datalist,
				    vector<modDatl>& modeldatalist,
				    list<kvStation>& slist,
				    int dateCol,
				    int ncp,
				    bool isShTy,
				    QString& userName)
{
  MDITabWindow* et = new MDITabWindow( ws, 
				       0, 
				       WDestructiveClose, 
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
  connect( et, SIGNAL( message(const QString&, int) ), statusBar(), 
	   SLOT( message(const QString&, int )) );
  if ( lity == erLi || lity == erSa ) {
    et->setCaption("Feilliste");
  }
  else if ( lity == erLo )
    et->setCaption("Feillog");
  else if ( lity == daLi )
    et->setCaption("Dataliste");
  et->setIcon( QPixmap("hqc.png") );
  //  et->showMaximized();
  et->show();
  if ( lity == erLi || lity == erSa ) {
    tileHorizontal();
  }
  //  ws->tile();
  vector<QString> stationList;
  int stnr=-1;
  for ( int i = 0; i < datalist.size(); i++) {
    QString name;
    double lat,lon,hoh;
    int env;
    int snr;
    if(stnr != datalist[i].stnr){
      stnr = datalist[i].stnr;
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
  cerr << "HqcMainWindow::closeWindow()\n";
    MDITabWindow* t = (MDITabWindow*)ws->activeWindow();
    if ( t )
	t->close();
}


void HqcMainWindow::help()
{
  
  QString path = QString(getenv("HQCDIR"));
  /*
  HelpTree *help = new HelpTree(path + "/docs/kvalobs.html", ".", 0, "help viewer");
  help->setCaption("Hqc hjelp");
  help->show();
  */
  system("mozilla " + path + "/docs/HqcFlaggkoder.html");
  //  system("mozilla http://kvalobs");
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
  QString type = "Diana";
  
  if(pluginB->connectClient(type)){
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
  
  QByteArray a;
  QDataStream s(a, IO_WriteOnly);
  s << image;
  
  miMessage m;
  m.command= qmstrings::addimage;
  m.description= "name:image";

  ostringstream ost;
  ost << name << ":";
  int n= a.count();
  for (int i=0; i<n; i++)
    ost << setw(7) << int(a[i]);
  miutil::miString txt= ost.str();
  m.data.push_back(txt);
  cerr << "HQC:     command:" << m.command << endl;
  cerr << "HQC: description:" << m.description << endl;
  cerr << "HQC sender melding" << endl;
  pluginB->sendMessage(m);
}

// called when client-list changes
void HqcMainWindow::processConnect()
{
  initDiana();
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
    m.data.push_back(datalist[i].otime.isoTime());
  }
  cerr << "HQC:     command:" << m.command << endl;
  cerr << "HQC:  commondesc:" << m.commondesc << endl;
  cerr << "HQC:      common:" << m.common << endl;
  cerr << "HQC: description:" << m.description << endl;
  cerr << "HQC sender melding" << endl;
  pluginB->sendMessage(m);
  
}

// processes incoming miMessages
void HqcMainWindow::processLetter(miMessage& letter)
{
  cerr << "Innkommende melding : " << letter.command.c_str() << endl;
  if(letter.command == qmstrings::newclient)
    processConnect(); 
  else if (letter.command == "station" ) {
    const char* ccmn = letter.common.c_str();
    QString cmn = QString(ccmn);
    cerr << "Innkommende melding: statTimeReceived is emitted."  << endl;
    emit(statTimeReceived(cmn));
    TimeseriesOK();
  }
  else if(letter.command == qmstrings::timechanged){
    miutil::miTime newTime(letter.common);
    sendObservations(newTime,false);
  }
}


// send text to show in text-window in diana
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


// send message to show ground analysis in Diana
bool  HqcMainWindow::sendAnalysisMessage() {

  //show analysis
  miMessage letter;
  letter.command="apply_quickmenu";
  letter.data.push_back("Hqc");
  letter.data.push_back("Bakkeanalyse");
  cerr <<"HQC: command:" << letter.command<<endl;
  for(int i=0;i<letter.data.size();i++)
    cerr <<"HQC    :data:" << letter.data[i]<<endl;
  cerr << "HQC sender melding" << endl;
  pluginB->sendMessage(letter);
  dianaTime.setTime(miutil::miString("2000-01-01 00:00:00"));
  return true;
}

void HqcMainWindow::sendStation(int stnr)
{

  miMessage pLetter;
  pLetter.command = "station";
  miutil::miString stationstr(stnr);
  pLetter.common = stationstr;
  cerr << "HQC: command:" << pLetter.command << endl;
  cerr << "HQC: common:" << pLetter.common << endl;
  cerr << "HQC sender melding" << endl;
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
    miMessage tLetter;
    tLetter.command = "settime";
    tLetter.commondesc = "time";
    tLetter.common = time.isoTime();
    cerr <<"HQC: command:" << tLetter.command << endl;
    cerr <<"HQC: commondesc:" << tLetter.commondesc << endl;
    cerr <<"HQC: common:" << tLetter.common << endl;
    cerr << "HQC sender melding" << endl;
    pluginB->sendMessage(tLetter);
  }
  miMessage pLetter;
  pLetter.command = "init_HQC_params";
  pLetter.commondesc = "time,plottype";

  
  //finding parameter names and indexes
  vector<int> parIndex;
  QStringList parName;
  // Using vector<bool> is not recomended - using deque instead
  //  vector<bool> parSynop;
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
  miutil::miString synopDescription = "id,St.type,auto,lon,lat,";
  if ( !parName.isEmpty() )
    synopDescription += parName.join(",").latin1();
  miutil::miString enkelDescription = "id,St.type,auto,lon,lat,";
  if ( !parName.isEmpty() )
    enkelDescription += parName.join(",").latin1();

  vector<miutil::miString> synopData;
  vector<miutil::miString> enkelData;

  for ( int i = 0; i < datalist.size(); i++) { // fill data
    if ( datalist[i].otime == time ){
      double lat,lon,h;
      miutil::miString str(datalist[i].stnr);
      QString name;
      int env;
      int snr;
      //      int typeId = datalist[i].typeId;
      int typeId = datalist[i].showTypeId;
      findStationInfo(datalist[i].stnr,name,lat,lon,h,snr,env);
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
      miutil::miString synopStr = str;
      miutil::miString enkelStr = str;
      double aa = datalist[i].corr[1];
      //      double tan12 = datalist[i].corr[214];
      //      double tax12 = datalist[i].corr[216];
      for(int j=0; j<parIndex.size();j++){
	double corr = datalist[i].corr[parIndex[j]];
	if ( parModel[j] ) {
	  for ( int k = 0; k < modeldatalist.size(); k++ ) {
	    if ( modeldatalist[k].stnr == datalist[i].stnr && 
		 modeldatalist[k].otime == datalist[i].otime ) {
	      corr = modeldatalist[k].orig[parIndex[j]];
	      break;
	    }
	  }
	}
	if ( parDiff[j] ) {
	  for ( int k = 0; k < modeldatalist.size(); k++ ) {
	    if ( modeldatalist[k].stnr == datalist[i].stnr && 
		 modeldatalist[k].otime == datalist[i].otime ) {
	      corr = corr - modeldatalist[k].orig[parIndex[j]];
	      break;
	    }
	  }
	}
	if ( parProp[j] ) {
	  for ( int k = 0; k < modeldatalist.size(); k++ ) {
	    if ( modeldatalist[k].stnr == datalist[i].stnr && 
		 modeldatalist[k].otime == datalist[i].otime ) {
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
	  int flag = datalist[i].flag[parIndex[j]];
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
	  synvalstr += ";";
	  synvalstr +=flagstr;
	  if(parSynop[j]){
	    enkelStr += ",";
	    enkelStr += valstr;
	    cerr << enkelStr << endl;
	    synopStr += ",";
	    synopStr += synvalstr;
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
    pLetter.description = synopDescription;
    pLetter.common = time.isoTime() + ",synop";
    pLetter.data = synopData;
    
    cerr <<"SYNOP"<<endl;
    cerr <<"HQC: command:"<<pLetter.command<<endl;
    cerr <<"HQC: commondesc:"<<pLetter.commondesc<<endl;
    cerr <<"HQC: common:"<<pLetter.common<<endl;
    cerr <<"HQC: desc:"<<pLetter.description<<endl;
    
    for(int i=0;i<pLetter.data.size();i++)
      cerr <<"HQC: data:"<<pLetter.data[i]<<endl;
    cerr << "HQC sender melding" << endl;
    pluginB->sendMessage(pLetter);
  }

  if(enkelData.size()){
    pLetter.description = enkelDescription;
    pLetter.common = time.isoTime() + ",enkel";
    pLetter.data = enkelData;
    
    cerr <<"ENKEL"<<endl;
    cerr <<"HQC: command:"<<pLetter.command<<endl;
    cerr <<"HQC: commondesc:"<<pLetter.commondesc<<endl;
    cerr <<"HQC: common:"<<pLetter.common<<endl;
    cerr <<"HQC: desc:"<<pLetter.description<<endl;
    
    for(int i=0;i<pLetter.data.size();i++)
      cerr <<"HQC: data:"<<pLetter.data[i]<<endl;
    cerr << "HQC sender melding" << endl;
    pluginB->sendMessage(pLetter);
  }

  miMessage okLetter;
  okLetter.command = "menuok";
  cerr <<"HQC: command:"<<okLetter.command<<endl;
  cerr << "HQC sender melding" << endl;
  pluginB->sendMessage(okLetter);
  
}



void HqcMainWindow::sendSelectedParam(miutil::miString param)
{
       
  param = dianaName(param);
  if(!param.exists()) return;

  miMessage pLetter;
  pLetter.command = "select_HQC_param";
  pLetter.commondesc = "param";
  pLetter.common = param;
  cerr << "HQC: command    = " << pLetter.command << endl;
  cerr << "HQC: commondesc = " << pLetter.commondesc << endl;
  cerr << "HQC: common     = " << pLetter.common << endl;
  cerr << "HQC sender melding" << endl;
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
  while( i<n && ( datalist[i].stnr!=station || datalist[i].otime != time)) i++;
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
  double cdValue = dianaValue(parameterIndex, false, atof(value.cStr()),datalist[i].corr[1]);
  int    iflag  = atoi(flag.cStr());
  datalist[i].corr[parameterIndex]=dValue;
  datalist[i].flag[parameterIndex]=iflag;
  miutil::miString value_flag = miutil::miString(cdValue) + ":" + flag;

  //update timeseries
  TimeseriesOK();

  //update diana
  miMessage pLetter;
  pLetter.command = "update_HQC_params";
  pLetter.commondesc = "time,plottype";
  pLetter.common = time.isoTime() + ",enkel";
  pLetter.description = "id," + param;
  miutil::miString data(station);
  data += ",";
  data += value_flag;
  pLetter.data.push_back(data);
  cerr << "HQC: command    = " << pLetter.command << endl;
  cerr << "HQC: commondesc = " << pLetter.commondesc << endl;
  cerr << "HQC: common     = " << pLetter.common << endl;
  cerr << "HQC: description= " << pLetter.description << endl;
  for(int i=0;i<pLetter.data.size();i++)
    cerr <<"HQC: data:"<<pLetter.data[i]<<endl;
  cerr << "HQC sender melding" << endl;
  pluginB->sendMessage(pLetter);

  pLetter.common = time.isoTime() + ",synop";
  miutil::miString dianaParam = dianaName(param);
  if(dianaParam.exists()){
    pLetter.description = "id," + dianaParam;
    cerr << "HQC: common     = " << pLetter.common << endl;
    cerr << "HQC: description= " << pLetter.description << endl;
  cerr << "HQC sender melding" << endl;
    pluginB->sendMessage(pLetter);
  }
}

// Help function to translate from qualobs parameter value to diana parameter value 
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
  //  file->setItemEnabled( filePrintMenuItem, enabled );
  cerr << "Save " << (enabled? "en":"dis") << "abled\n";
}

int HqcMainWindow::findTypeId(int typ, int pos, int par, miutil::miTime oTime)
{
  int tpId;
  tpId = typ;
  //  for(CIObsPgmList obit=obsPgmList.begin();obit!=obsPgmList.end(); obit++){
  for(CIObsPgmList obit=obsPgmList.end();obit!=obsPgmList.begin(); obit--){
    if ( obit->stationID() == pos && obit->paramID() == par && obit->fromtime() < oTime) {
      tpId = obit->typeID();
      break;
    }
  }
  if ( tpId == -32767 ) {
    switch (par) {
    case 106:
      for(CIObsPgmList obit=obsPgmList.end();obit!=obsPgmList.begin(); obit--){
	if ( obit->stationID() == pos && obit->paramID() == 105 && obit->fromtime() < oTime) {
	  tpId = obit->typeID();
	  break;
	}
      }
      break;
    case 109:
      for(CIObsPgmList obit=obsPgmList.end();obit!=obsPgmList.begin(); obit--){
	if ( obit->stationID() == pos && (obit->paramID() == 104 || obit->paramID() == 105 || obit->paramID() == 106) && obit->fromtime() < oTime) {
	  tpId = obit->typeID();
	  break;
	}
      }
      break;
    case 110:
      for(CIObsPgmList obit=obsPgmList.end();obit!=obsPgmList.begin(); obit--){
	if ( obit->stationID() == pos && (obit->paramID() == 104 || obit->paramID() == 105 || obit->paramID() == 106 || obit->paramID() == 109) && obit->fromtime() < oTime) {
	  tpId = obit->typeID();
	  break;
	}
      }
      break;
    case 214:
      for(CIObsPgmList obit=obsPgmList.end();obit!=obsPgmList.begin(); obit--){
	if ( obit->stationID() == pos && obit->paramID() == 213 && obit->fromtime() < oTime) {
	  tpId = obit->typeID();
	  break;
	}
      }
      break;
    case 216:
      for(CIObsPgmList obit=obsPgmList.end();obit!=obsPgmList.begin(); obit--){
	if ( obit->stationID() == pos && obit->paramID() == 215 && obit->fromtime() < oTime) {
	  tpId = obit->typeID();
	  break;
	}
      }
      break;
    case 224:
      for(CIObsPgmList obit=obsPgmList.end();obit!=obsPgmList.begin(); obit--){
	if ( obit->stationID() == pos && obit->paramID() == 223 && obit->fromtime() < oTime) {
	  tpId = obit->typeID();
	  break;
	}
      }
      break;
    default:;
    }
  }
  return tpId;
}
