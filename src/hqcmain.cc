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
#include "KvalobsDataModel.h"
#include "KvalobsDataView.h"
//#include "discardbox.h"
#include "discarddialog.h"
#include "approvedialog.h"
#include "connect2stinfosys.h"
#include <iomanip>
#include <QAction>
#include <qpixmap.h>
#include <QWindowsXPStyle>
#include <qwindowsstyle.h>
#include <qcdestyle.h>
#include <qcommonstyle.h>
#include <qvalidator.h>
#include <qmetaobject.h>
#include <qlistview.h>
#include <QFrame>
#include <QTextStream>
#include <qTimeseries/TSPlot.h>
#include <glText/glTextQtTexture.h>
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
#include <QMdiArea>
#include <QMdiSubWindow>
#include <glText/glTextQtTexture.h>
#include <boost/assign.hpp>
#include <QDateTimeEdit>
#include <QSizePolicy>
#include <QSqlQuery>
#include <QtGui>
#include <QList>

using namespace std;

namespace {
  // Number of parameters to show, according to last selection
  //int noSelPar;
  const int modelParam[] =
    { 61, 81, 109, 110, 177, 178, 211, 262 };
  miutil::miTime remstime;
  miutil::miTime remetime;
  miutil::miTime remdlstime;
  miutil::miTime remdletime;
  listType remLity;

const std::map<QString, QString> configNameToUserName = boost::assign::map_list_of
        ("[airpress]", "Lufttrykk")
        ("[temperature]", "Temperatur")
        ("[prec]", "Nedbør")
        ("[visual]", "Visuell")
        ("[wave]", "Sjøgang")
        ("[synop]", "Synop")
        ("[klstat]", "Klimastatistikk")
        ("[priority]", "Prioriterte parametere")
        ("[wind]", "Vind")
        ("[plu]", "Pluviometerkontroll")
        ("[all]", "Alt");
}

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
  : QMainWindow( 0, "HQC")
  , reinserter( NULL )
  , datalist(new model::KvalobsDataList)
{
  /////TEST
  //  readSettings();
  /////TEST SLUTT
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

  // ---- ACTIONS -----------------------------------------------

  QPixmap icon_listdlg("/usr/local/etc/kvhqc/table.png");
  QAction * dataListAction = new QAction(icon_listdlg, tr("&Dataliste"), this);
  dataListAction->setShortcut(Qt::CTRL+Qt::Key_D);
  connect(dataListAction, SIGNAL(activated()), this, SLOT(dataListMenu()));

  QPixmap icon_ts("/usr/local/etc/kvhqc/kmplot.png");
  QAction * timeSeriesAction = new QAction(icon_ts, tr("&Tidsserie"), this);
  connect(timeSeriesAction, SIGNAL(activated()), this, SLOT(timeseriesMenu()));

  QAction * timesAction = new QAction(tr("&Tidspunkter"), this);
  timesAction->setShortcut(Qt::CTRL | Qt::Key_T);
  connect(timesAction, SIGNAL(activated()), this, SLOT(clk()));

  //  lackListAction = new QAction(tr("&Mangelliste"), this);
  //  lackListAction->setShortcut(Qt::CTRL+Qt::Key_M);
  // ---- Workspace ---------------------------------------------
  ws = new QMdiArea(this);
  setCentralWidget( ws );

  connect( ws, SIGNAL(subWindowActivated(QMdiSubWindow*)),
           this, SLOT(updateSaveFunction(QMdiSubWindow*)) );



  // ---- MAIN MENU ---------------------------------------------
  //  qApp->setStyle(new QSGIStyle);

  listExist = FALSE;

  QMenu * file = menuBar()->addMenu(tr("&Fil"));
  saveAction = file->addAction( "Lagre", this, SIGNAL( saveData() ), Qt::CTRL+Qt::Key_S );
  saveAction->setEnabled(false);
  printAction = file->addAction( "Skriv ut", this, SIGNAL( printErrorList() ), Qt::CTRL+Qt::Key_P );
  printAction->setEnabled(false);

  file->addAction( "&Lukk",    ws, SLOT(closeActiveSubWindow()), Qt::CTRL+Qt::Key_W );
  file->addAction( "&Avslutt", qApp, SLOT( closeAllWindows() ));


  choice = menuBar()->addMenu(tr("&Valg"));
  flID = choice->addAction( "Vis &flagg",                 this, SLOT(showFlags()));
  flID->setCheckable(true);
  flID->setChecked(false);
  orID = choice->addAction( "Vis &original",              this, SLOT(showOrigs()));
  orID->setCheckable(true);
  orID->setChecked(true);
  moID = choice->addAction( "Vis &modeldata",             this, SLOT(showMod()));
  moID->setCheckable(true);
  moID->setChecked(true);
  stID = choice->addAction( "Vis &stasjonsnavn",          this, SLOT(showStat()));
  stID->setCheckable(true);
  stID->setChecked(true);
  heID = choice->addAction( "Vis s&tasjonshøyde", this, SLOT(showHeight()));
  heID->setCheckable(true);
  heID->setChecked(true);
  poID = choice->addAction( "Vis &posisjon", this, SLOT(showPos()));
  poID->setCheckable(true);
  poID->setChecked(false);

  //isShTy;


//  choice->setItemChecked(flID, isShFl);
//  choice->setItemChecked(orID, isShOr);

  
  QMenu * showmenu = new QMenu( this );
  menuBar()->insertItem( "&Listetype", showmenu);
  showmenu->addAction( "Data&liste og Feilliste    ", this, SLOT(allListMenu()),Qt::CTRL+Qt::Key_L );
  showmenu->addAction( "&Feilliste    ", this, SLOT(errListMenu()),Qt::CTRL+Qt::Key_F );
  showmenu->addAction( "F&eillog    ",   this, SLOT(errLogMenu()),Qt::CTRL+Qt::Key_E );
  showmenu->addAction(dataListAction);
  showmenu->addAction( "&Feilliste salen", this, SLOT(errLisaMenu()),Qt::ALT+Qt::Key_S );
  showmenu->insertSeparator();
  showmenu->addAction( "&Nedbør", this, SLOT( showWatchRR() ), Qt::CTRL+Qt::Key_R );
  showmenu->addAction( "&Vær", this, SLOT( showWeather() ), Qt::CTRL+Qt::Key_V );
  showmenu->insertSeparator();
  showmenu->addAction(timeSeriesAction);
  showmenu->insertSeparator();
  showmenu->addAction( "Te&xtData    ", this, SLOT(textDataMenu()),Qt::ALT+Qt::Key_X );
  showmenu->addAction( "Re&jected    ", this, SLOT(rejectedMenu()),Qt::CTRL+Qt::Key_J );
  
  QMenu * weathermenu = new QMenu( this );
  menuBar()->insertItem( "Vær&element", weathermenu);
  //  wElement = "";
  klID = weathermenu->addAction( "For &daglig rutine",       this, SLOT(climateStatistics()) );
  piID = weathermenu->addAction( "&Prioriterte parametere",  this, SLOT(priority()) );
  taID = weathermenu->addAction( "&Temperatur og fuktighet", this, SLOT(temperature()) );
  prID = weathermenu->addAction( "&Nedbør og snøforhold",    this, SLOT(precipitation()) );
  apID = weathermenu->addAction( "&Lufttrykk og vind",       this, SLOT(airPress()) );
  clID = weathermenu->addAction( "&Visuelle parametere",     this, SLOT(visuals()) );
  seID = weathermenu->addAction( "&Maritime parametere",     this, SLOT(sea()) );
  syID = weathermenu->addAction( "&Synop",                   this, SLOT(synop()) );
  wiID = weathermenu->addAction( "&Vind",                    this, SLOT(wind()) );
  plID = weathermenu->addAction( "&Pluviometerparametere",   this, SLOT(plu()) );
  alID = weathermenu->addAction( "&Alt",                     this, SLOT(all()) );


  QMenu * clockmenu = menuBar()->addMenu("&Tidspunkter");
  clockmenu->addAction(timesAction);
  
  menuBar()->insertItem( "&Forkast tidsserie", this, SLOT(rejectTimeseries()));
  menuBar()->insertItem( "&Godkjenn tidsserie", this, SLOT(acceptTimeseries()));
  
  menuBar()->insertItem( "&Dianavisning", this, SLOT(dsh()));
  menuBar()->insertItem( "&Kro", this, SLOT(startKro()));

  QMenu * help = new QMenu( this );
  menuBar()->insertItem( "&Hjelp", help );
  help->addAction( "&Brukerveiledning", this, SLOT(helpUse()), Qt::Key_F1);
  help->addAction( "&Flagg", this, SLOT(helpFlag()), Qt::Key_F2);
  help->addAction( "&Parametere", this, SLOT(helpParam()), Qt::Key_F3);
  help->insertSeparator();
  help->addAction( "&Om Hqc", this, SLOT(about()));
  help->insertSeparator();
  help->addAction( "Om &Qt", this, SLOT(aboutQt()));
  
  // --- MAIN WINDOW -----------------------------------------

  
  // --- TOOL BAR --------------------------------------------
  QToolBar * hqcTools = addToolBar("Hqcfunksjoner");
  hqcTools->addAction(dataListAction);
  hqcTools->addAction(timeSeriesAction);

  
  // --- STATUS BAR -------------------------------------------
  
  //  if(usesocket){
  miutil::miString name = "hqc";   
  //  miutil::miString command = "coserver4";
  miutil::miString command = "/usr/bin/coserver4";
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
    if ( listStatCoast[noStat] == "K" ) {
      coastStations.push_back(listStatNum[noStat].toInt());
    }
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
  actsdlg = new AcceptTimeseriesDialog();
  actsdlg->hide();
  rjtsdlg = new RejectTimeseriesDialog();
  rjtsdlg->hide();
 
  // --- READ PARAMETER INFO ---------------------------------------

  /////TEST
  readSettings();
  /////TEST SLUTT

  
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

  connect( rjtsdlg, SIGNAL(tsRejectApply()), SLOT(rejectTimeseriesOK()));
  connect( rjtsdlg, SIGNAL(tsRejectHide()), SLOT(rejectTimeseries()));

  connect( actsdlg, SIGNAL(tsAcceptApply()), SLOT(acceptTimeseriesOK()));
  connect( actsdlg, SIGNAL(tsAcceptHide()), SLOT(acceptTimeseries()));

  connect( this,SIGNAL(newStationList(std::vector<QString>&)), 
	   rjtsdlg,SLOT(newStationList(std::vector<QString>&)));
  connect( this,SIGNAL(newParameterList(const QStringList&)), 
	   rjtsdlg,SLOT(newParameterList(const QStringList&)));

  connect( this,SIGNAL(newStationList(std::vector<QString>&)), 
	   actsdlg,SLOT(newStationList(std::vector<QString>&)));
  connect( this,SIGNAL(newParameterList(const QStringList&)), 
	   actsdlg,SLOT(newParameterList(const QStringList&)));

  connect( lstdlg, SIGNAL(fromTimeChanged(const QDateTime&)),
	   tsdlg,  SLOT(setFromTimeSlot(const QDateTime&)));
  //	   tsdlg,  SLOT(setFromTimeSlot(const miutil::miTime&)));

  connect( lstdlg, SIGNAL(toTimeChanged(const QDateTime&)),
	   tsdlg,  SLOT(setToTimeSlot(const QDateTime&)));
  //	   tsdlg,  SLOT(setToTimeSlot(const miutil::miTime&)));

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
}

void HqcMainWindow::showOrigs() {
}

void HqcMainWindow::showMod() {
}

void HqcMainWindow::showStat() {
}

void HqcMainWindow::showPos() {
}

void HqcMainWindow::showHeight() {
}

void HqcMainWindow::showTyp() {
}

void HqcMainWindow::airPress() {
  wElement = "Lufttrykk";
  lity = daLi;
  firstObs = true;
  /*
  apID->setChecked(TRUE);
  taID->setChecked(FALSE);
  wiID->setChecked(FALSE);
  prID->setChecked(FALSE);
  clID->setChecked(FALSE);
  seID->setChecked(FALSE);
  syID->setChecked(FALSE);
  klID->setChecked(FALSE);
  piID->setChecked(FALSE);
  alID->setChecked(FALSE);
  plID->setChecked(FALSE);
  */
  const std::vector<int> & parameters = parameterGroups[wElement];
  Q_ASSERT(not parameters.empty());
  pardlg->insertParametersInListBox(parameters, parMap);
  pardlg->showAll();
  //  sendObservations(remstime,false);
}

void HqcMainWindow::temperature() {
  wElement = "Temperatur";
  lity = daLi;
  firstObs = true;
  /*
  apID->setChecked(FALSE);
  taID->setChecked(TRUE);
  wiID->setChecked(FALSE);
  prID->setChecked(FALSE);
  clID->setChecked(FALSE);
  seID->setChecked(FALSE);
  syID->setChecked(FALSE);
  klID->setChecked(FALSE);
  piID->setChecked(FALSE);
  alID->setChecked(FALSE);
  plID->setChecked(FALSE);
  */
    const std::vector<int> & parameters = parameterGroups[wElement];
  Q_ASSERT(not parameters.empty());
  pardlg->insertParametersInListBox(parameters, parMap);
  pardlg->showAll();
  //  sendObservations(remstime,false);
}

void HqcMainWindow::precipitation() {
  wElement = "Nedbør";
  lity = daLi;
  firstObs = true;
  /*
  apID->setChecked(FALSE);
  taID->setChecked(FALSE);
  wiID->setChecked(FALSE);
  prID->setChecked(TRUE);
  clID->setChecked(FALSE);
  seID->setChecked(FALSE);
  syID->setChecked(FALSE);
  klID->setChecked(FALSE);
  piID->setChecked(FALSE);
  plID->setChecked(FALSE);
  alID->setChecked(FALSE);
  */
  //  sendObservations(remstime,false);
  const std::vector<int> & parameters = parameterGroups[wElement];
  Q_ASSERT(not parameters.empty());
  pardlg->insertParametersInListBox(parameters, parMap);

  pardlg->showAll();
}

void HqcMainWindow::visuals() {
  wElement = "Visuell";
  lity = daLi;
  firstObs = true;
  /*
  apID->setChecked(FALSE);
  taID->setChecked(FALSE);
  wiID->setChecked(FALSE);
  prID->setChecked(FALSE);
  clID->setChecked(TRUE);
  seID->setChecked(FALSE);
  syID->setChecked(FALSE);
  klID->setChecked(FALSE);
  piID->setChecked(FALSE);
  alID->setChecked(FALSE);
  plID->setChecked(FALSE);
  */
  //  sendObservations(remstime,false);
  const std::vector<int> & parameters = parameterGroups[wElement];
  Q_ASSERT(not parameters.empty());
  pardlg->insertParametersInListBox(parameters, parMap);

  pardlg->showAll();
}


void HqcMainWindow::sea() {
  wElement = "Sjøgang";
  lity = daLi;
  /*
  apID->setChecked(FALSE);
  taID->setChecked(FALSE);
  wiID->setChecked(FALSE);
  prID->setChecked(FALSE);
  clID->setChecked(FALSE);
  seID->setChecked(TRUE);
  syID->setChecked(FALSE);
  klID->setChecked(FALSE);
  piID->setChecked(FALSE);
  alID->setChecked(FALSE);
  plID->setChecked(FALSE);
  */
   //  sendObservations(remstime,false);
   const std::vector<int> & parameters = parameterGroups[wElement];
  Q_ASSERT(not parameters.empty());
  pardlg->insertParametersInListBox(parameters, parMap);

  pardlg->showAll();
}

void HqcMainWindow::synop() {
  wElement = "Synop";
  lity = daLi;
  firstObs = true;
  /*
  apID->setChecked(FALSE);
  taID->setChecked(FALSE);
  wiID->setChecked(FALSE);
  prID->setChecked(FALSE);
  clID->setChecked(FALSE);
  seID->setChecked(FALSE);
  syID->setChecked(TRUE);
  klID->setChecked(FALSE);
  piID->setChecked(FALSE);
  alID->setChecked(FALSE);
  plID->setChecked(FALSE);
  */
  //  sendObservations(remstime,false);
  const std::vector<int> & parameters = parameterGroups[wElement];
  Q_ASSERT(not parameters.empty());
  pardlg->insertParametersInListBox(parameters, parMap);

  pardlg->showAll();
}

void HqcMainWindow::climateStatistics() {
  wElement = "Klimastatistikk";
  lity = daLi;
  firstObs = true;
  /*
  apID->setChecked(FALSE);
  taID->setChecked(FALSE);
  wiID->setChecked(FALSE);
  prID->setChecked(FALSE);
  clID->setChecked(FALSE);
  seID->setChecked(FALSE);
  syID->setChecked(FALSE);
  //  klID->setChecked(true);
  piID->setChecked(FALSE);
  alID->setChecked(FALSE);
  plID->setChecked(FALSE);
  */
  //  sendObservations(remstime,false);
    const std::vector<int> & parameters = parameterGroups[wElement];
  Q_ASSERT(not parameters.empty());
  pardlg->insertParametersInListBox(parameters, parMap);
  pardlg->showAll();
}

void HqcMainWindow::priority() {
  wElement = "Prioriterte parametere";
  lity = daLi;
  firstObs = true;
  /*
  apID->setChecked(FALSE);
  taID->setChecked(FALSE);
  wiID->setChecked(FALSE);
  prID->setChecked(FALSE);
  clID->setChecked(FALSE);
  seID->setChecked(FALSE);
  syID->setChecked(FALSE);
  klID->setChecked(FALSE);
  piID->setChecked(TRUE);
  alID->setChecked(FALSE);
  plID->setChecked(FALSE);
  */
  //  sendObservations(remstime,false);
    const std::vector<int> & parameters = parameterGroups[wElement];
  Q_ASSERT(not parameters.empty());
  pardlg->insertParametersInListBox(parameters, parMap);

  pardlg->showAll();
}

void HqcMainWindow::wind() {
  wElement = "Vind";
  lity = daLi;
  firstObs = true;
  /*
  apID->setChecked(FALSE);
  taID->setChecked(FALSE);
  wiID->setChecked(TRUE);
  prID->setChecked(FALSE);
  clID->setChecked(FALSE);
  seID->setChecked(FALSE);
  syID->setChecked(FALSE);
  klID->setChecked(FALSE);
  piID->setChecked(FALSE);
  alID->setChecked(FALSE);
  plID->setChecked(FALSE);
  */
  //  sendObservations(remstime,false);
  const std::vector<int> & parameters = parameterGroups[wElement];
  Q_ASSERT(not parameters.empty());
  pardlg->insertParametersInListBox(parameters, parMap);

  pardlg->showAll();
}

void HqcMainWindow::plu() {
  wElement = "Pluviometerkontroll";
  lity = daLi;
  firstObs = true;
  /*
  apID->setChecked(FALSE);
  taID->setChecked(FALSE);
  wiID->setChecked(FALSE);
  prID->setChecked(FALSE);
  clID->setChecked(FALSE);
  seID->setChecked(FALSE);
  syID->setChecked(FALSE);
  klID->setChecked(FALSE);
  piID->setChecked(FALSE);
  alID->setChecked(FALSE);
  plID->setChecked(TRUE);
  */
  //  sendObservations(remstime,false);
  const std::vector<int> & parameters = parameterGroups[wElement];
  Q_ASSERT(not parameters.empty());
  pardlg->insertParametersInListBox(parameters, parMap);

  pardlg->showAll();
}

void HqcMainWindow::all() {
  wElement = "Alt";
  lity = daLi;
  firstObs = true;
  /*
  apID->setChecked(FALSE);
  taID->setChecked(FALSE);
  wiID->setChecked(FALSE);
  prID->setChecked(FALSE);
  clID->setChecked(FALSE);
  seID->setChecked(FALSE);
  syID->setChecked(FALSE);
  klID->setChecked(FALSE);
  piID->setChecked(FALSE);
  plID->setChecked(FALSE);
  alID->setChecked(TRUE);
  */
  //  sendObservations(remstime,false);
  const std::vector<int> & parameters = parameterGroups[wElement];
  Q_ASSERT(not parameters.empty());
  pardlg->insertParametersInListBox(parameters, parMap);

  pardlg->showAll();
}

void HqcMainWindow::paramOK

() {
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

void HqcMainWindow::saveDataToKvalobs(const kvalobs::kvData & toSave)
{
  if ( ! reinserter ) {
    qDebug("Skipping data save, since user is not authenticated");
    return;
  }
  kvalobs::serialize::KvalobsData dataList;
  dataList.insert(toSave);
  const CKvalObs::CDataSource::Result_var result = reinserter->insert(dataList);
  switch (result->res )
  {
  case OK:
     break;
   // Insert any custom messages here
  default:
    QMessageBox::critical(this, "Unable to insert",
        QString("An error occured when attempting to insert data into kvalobs.\n"
        "The message from kvalobs was\n\n") + QString(result->message),
        QMessageBox::Ok, QMessageBox::NoButton);
    return;
  }
}

namespace
{
class FunctionLogger
{
  const std::string name_;

  static int indent;

  void log_(const std::string & msg) const
  {
    std::ostringstream data;
    for ( int i = 0; i < indent; ++ i )
      data << "--------";
    data << "> " << msg.c_str();
    std::string out = data.str();
    qDebug() << out.c_str();
  }

public:
  FunctionLogger(const char * name) : name_(name)
  {
    ++ indent;
    log_("Entering " + name_);
  }
  ~FunctionLogger()
  {
    log_("Leaving  " + name_);
    -- indent;
  }
};
int FunctionLogger::indent = 3;
}
#define LOG_FUNCTION() FunctionLogger INTERNAL_function_logger(__func__)

void HqcMainWindow::ListOK() {
  LOG_FUNCTION();
  if ( !dianaconnected ) {
    int dianaWarning = QMessageBox::warning(this, 
					    "Dianaforbindelse",
					    "Diana er ikke koplet til!"
					    "ønsker du å kople til Diana?",
					    "&Ja",
					    "&Nei");
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

  miutil::miTime stime; // start time
  stime.setTime(miutil::miString(lstdlg->getStart().latin1()));

  miutil::miTime etime; // end time
  etime.setTime(miutil::miString(lstdlg->getEnd().latin1()));
 
  QMap<QString, std::vector<int> >::const_iterator find = parameterGroups.find(wElement);
  if ( find ==  parameterGroups.end() ) {
      QMessageBox::critical(this, "Internal error",
          "Configuration file does not seem to have an entry for parameter group " + wElement,
          QMessageBox::Ok, QMessageBox::NoButton);
      return;
  }

  parFind = find->size();
  std::vector<int> parameterList;

  selPar.clear();
  int kk = 0;

  for ( int jj = 0; jj < find->size(); jj++ ) {
    int paramIndex = (*find)[jj];
    const QString & sp =  parMap[paramIndex];
    
    bool found = pardlg->plb->item(jj)->isSelected();
    qDebug() << qPrintable(pardlg->plb->item(jj)->text()) << ": " << found << " (" << pardlg->markPar->isChecked() << "" << pardlg->noMarkPar->isChecked() << "" << pardlg->allPar->isChecked() << ")";
    if ( (found && pardlg->markPar->isChecked()) ||
	 (!found && pardlg->noMarkPar->isChecked()) ||
	 pardlg->allPar->isChecked() ) {
      selParNo[kk] = paramIndex;
      selPar.append(sp);
      kk++;
      parameterList.push_back(paramIndex);
    }
  }
  for ( std::vector<int>::const_iterator it = parameterList.begin(); it != parameterList.end(); ++ it )
    qDebug() << "Selected: "<< * it;

  isShTy = lstdlg->allTypes->isChecked();

  readFromData(stime, etime, lity);
  readFromModelData(stime, etime);

  // All windows are shown later, in the tileHorizontal function

  if ( lity == daLi or lity == alLi ) {

      model::KvalobsDataView * tableView = new model::KvalobsDataView(modelParam, modelParam + NOPARAMMODEL, this);
      tableView->setAttribute(Qt::WA_DeleteOnClose);

//      model::KvalobsDataDelegate * delegate = new model::KvalobsDataDelegate(tableView);
//      tableView->setItemDelegate(delegate);

//      model::KvalobsDataModel * dataModel =
      dataModel =
          new model::KvalobsDataModel(
              parameterList, parMap, datalist, modeldatalist,
              stID->isChecked(), poID->isChecked(), heID->isChecked(),
              reinserter != 0,
              tableView);

      connect(dataModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(TimeseriesOK()));
      connect(dataModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(sendAnalysisMessage()));
      connect(dataModel, SIGNAL(dataModification(const kvalobs::kvData &)), this, SLOT(saveDataToKvalobs(const kvalobs::kvData &)));

      // Functionality for hiding/showing rows in data list
      connect(flID, SIGNAL(toggled(bool)), tableView, SLOT(toggleShowFlags(bool)));
      connect(orID, SIGNAL(toggled(bool)), tableView, SLOT(toggleShowOriginal(bool)));
      connect(moID, SIGNAL(toggled(bool)), tableView, SLOT(toggleShowModelData(bool)));

      connect(this, SIGNAL(statTimeReceived(const QString &)), tableView, SLOT(selectStation(const QString &)));

      connect(tableView, SIGNAL(stationSelected(int, const miutil::miTime &)), this, SLOT(sendStation(int)));
      connect(tableView, SIGNAL(timeSelected(const miutil::miTime &)), SLOT(sendObservations(const miutil::miTime &)));
      connect(tableView, SIGNAL(parameterSelected(const QString &)), SLOT(sendSelectedParam(const QString &)));
      //connect(tableView, SIGNAL(newSelection()), SLOT(updateParams(int, const miutil::miTime &, const miutil::miString &,const miutil::miString & value,const miutil::miString & flag))))

      connect(stID, SIGNAL(toggled(bool)), dataModel, SLOT(setShowStationName(bool)));
      connect(poID, SIGNAL(toggled(bool)), dataModel, SLOT(setShowPosition(bool)));
      connect(heID, SIGNAL(toggled(bool)), dataModel, SLOT(setShowHeight(bool)));

      tableView->setModel(dataModel);

      // Smaller cells. This should probably be done dynamically, somehow
      int columns = dataModel->columnCount();
      for ( int i = 0; i < columns; ++ i )
        tableView->setColumnWidth(i, 56);

      int rows = dataModel->rowCount();
      for ( int i = 0; i < rows; ++ i )
        tableView->setRowHeight(i,24);

      tableView->toggleShowFlags(flID->isChecked());
      tableView->toggleShowOriginal(orID->isChecked());
      tableView->toggleShowModelData(moID->isChecked());

      ws->addSubWindow(tableView);

      tableView->setIcon( QPixmap("/usr/local/etc/kvhqc/hqc.png") );
      tableView->setCaption("Dataliste");
  }

  if ( lity == erLi or lity == erSa or lity == alLi ) {
//      Q_ASSERT(metty != tabHead); // deprecated functionality
//      Q_ASSERT(metty == tabList);
//      metty may be undefined here

      int dateCol = 0;
      if ( poID->isChecked() && !stID->isChecked() )
        dateCol = 1;
      else if ( !poID->isChecked() && stID->isChecked() )
        dateCol = 2;
      else if ( poID->isChecked() && stID->isChecked() )
        dateCol = 3;

      int ncp;
      if ( !orID->isChecked() && !flID->isChecked() && !moID->isChecked() )
        ncp = 0;
      if ( orID->isChecked() && !flID->isChecked() && !moID->isChecked() )
        ncp = 1;
      if ( !orID->isChecked() && flID->isChecked() && !moID->isChecked() )
        ncp = 2;
      if ( orID->isChecked() && flID->isChecked() && !moID->isChecked() )
        ncp = 3;
      if ( !orID->isChecked() && !flID->isChecked() && moID->isChecked() )
        ncp = 4;
      if ( orID->isChecked() && !flID->isChecked() && moID->isChecked() )
        ncp = 5;
      if ( !orID->isChecked() && flID->isChecked() && moID->isChecked() )
        ncp = 6;
      if ( orID->isChecked() && flID->isChecked() && moID->isChecked() )
        ncp = 7;

      ErrorList * erl = new ErrorList(selPar,
                          stime,
                          etime,
                          0, // unused
                          0, // unused
                          this,
                          lity,
                          metty,
                          selParNo,
                          *datalist,
                          modeldatalist,
                          slist,
                          dateCol,
                          ncp,
                          userName);
      model::KvalobsDataView * tableView = new model::KvalobsDataView(modelParam, modelParam + NOPARAMMODEL, this);
      tableView->setAttribute(Qt::WA_DeleteOnClose);
      connect(saveAction, SIGNAL( activated() ), erl, SLOT( saveChanges() ) );
      //      connect( erl, SIGNAL( stationSelected( int, const miutil::miTime & ) ), tableView, SLOT(selectStation(const QString &)));
      connect( erl, SIGNAL( statSel( miMessage& ) ),
	       SLOT( processLetter( miMessage &) ) );
      ws->addSubWindow(erl);
  }

  tileHorizontal();

  vector<QString> stationList;
  int stnr=-1;
  for ( int i = 0; i < datalist->size(); i++) {
    QString name;
    double lat,lon,hoh;
    int env;
    int snr;
    if(stnr != (*datalist)[i].stnr()){
      stnr = (*datalist)[i].stnr();
      findStationInfo(stnr,name,lat,lon,hoh,snr,env);
      QString nrStr;
      nrStr = nrStr.setNum(stnr);
      QString statId = nrStr + " " + name;
      stationList.push_back(statId);
    }
  }
  emit newStationList(stationList);
  cerr << "newStationList emitted " << endl;


  //  send parameter names to ts dialog
  emit newParameterList(selPar);
  if ( lity != erLi && lity != erSa  ) {
    sendAnalysisMessage();
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
      for ( int i = 0; i < datalist->size(); i++) { // fill data
	if ( (*datalist)[i].stnr() == stationIndex[ip] &&
	     (*datalist)[i].otime() >= stime &&
             (*datalist)[i].otime() <= etime &&
	     (*datalist)[i].otime().min() == 0 ) {
	  if ( (*datalist)[i].corr(parameterIndex[ip]) > -32766.0 )
	    tseries.add(TimeSeriesData::Data((*datalist)[i].otime(),
					     (*datalist)[i].corr(parameterIndex[ip])));
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
					  (*datalist)[i].corr(parameterIndex[ip])
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
  readFromStation();
  if ( !readFromStInfoSys() ) {
    int statCheck = 0;
    readFromStationFile(statCheck);
  }
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
  GetTextData textDataReceiver(this);
  if(!KvApp::kvApp->getKvData(textDataReceiver, whichData)){
    //cerr << "Finner ikke  textdatareceiver!!" << endl;
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
  QMdiSubWindow * subWindow = ws->activeSubWindow();

  kvalobs::kvData data;

  // TODO: Reinstate this
//  if ( subWindow ) {
//    MDITabWindow * current = dynamic_cast<MDITabWindow *>(subWindow->widget());
//    if ( current and current->dtt )
//      data = current->dtt->getKvData();
    ErrorList * errorList = dynamic_cast<ErrorList *>(subWindow->widget());
    if ( errorList )
      data = errorList->getKvData();
//  }

  WatchRR::RRDialog * rrd = WatchRR::RRDialog::getRRDialog( data, slist, this, Qt::Window );
  if ( rrd ) {
    rrd->setReinserter( reinserter );
    rrd->show();
  }
}

void HqcMainWindow::showWeather()
{
  QMdiSubWindow * subWindow = ws->activeSubWindow();

  kvalobs::kvData data;

  // TODO: Reinstate this
    if ( subWindow ) {
//    MDITabWindow * current = dynamic_cast<MDITabWindow *>(subWindow->widget());
//    if ( current and current->dtt )
//      data = current->dtt->getKvData();
    ErrorList * errorList = dynamic_cast<ErrorList *>(subWindow->widget());
    if ( errorList )
      data = errorList->getKvData();
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
    QDateTime mx = QDateTime::currentDateTime();
    int noDays = lstdlg->toTime->date().dayOfYear() - lstdlg->fromTime->date().dayOfYear();
    if ( noDays < 0 ) noDays = 365 + noDays;
        
    if ( noDays < 27 ) {
      mx = mx.addSecs(-60*mx.time().minute());
      mx = mx.addSecs(3600);
      lstdlg->toTime->setDateTime(mx);
    }
    
    lstdlg->showAll();
  }
}
/*
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
*/
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

void HqcMainWindow::rejectTimeseries() {
  if ( rjtsdlg->isVisible() ) {
    rjtsdlg->hideAll();
  } else {
    rjtsdlg->showAll();
  }
}

void HqcMainWindow::acceptTimeseries() {
  if ( actsdlg->isVisible() ) {
    actsdlg->hideAll();
  } else {
    actsdlg->showAll();
  }
}

void HqcMainWindow::acceptTimeseriesOK() {
  QDateTime stime;
  QDateTime etime;
  QString parameter;
  int stationIndex;
  int parameterIndex;
  bool result = actsdlg->getResults(parameter,stime,etime,stationIndex);
  if ( !result ) return;
  std::list<kvalobs::kvParam>::const_iterator it=plist.begin();
  for(;it!=plist.end(); it++){
    if ( it->name().cStr() == parameter ){
      parameterIndex = it->paramID();
      break;
    }
  }
  WhichDataHelper whichData;
  long int stnr = stationIndex;
  miutil::miTime ft;
  miutil::miTime tt;
  ft.setTime(miutil::miString(stime.toString("yyyy-MM-dd hh:mm:ss").toStdString()));
  tt.setTime(miutil::miString(etime.toString("yyyy-MM-dd hh:mm:ss").toStdString()));
  whichData.addStation(stnr, ft, tt);
  checkTypeId(stnr);
  int firstRow = dataModel->dataRow(stnr, ft);
  int lastRow  = dataModel->dataRow(stnr, tt);
  int column   = dataModel->dataColumn(parameter);

  QString ch;
  vector<QString> chList;
  vector<double> newCorr;
  for ( int irow = firstRow; irow <= lastRow; irow++) {
    QModelIndex index = dataModel->index(irow, column);
    const kvData & dt = dataModel->getKvData_(index);
    //    if ( dt.corrected() < -32760 )
    //      continue;
    QString ori;
    ori = ori.setNum(dt.original(), 'f', 1); 
    QString stnr;
    stnr = stnr.setNum(dt.stationID());
    ch = stnr + " " + QString(dt.obstime().isoTime().cStr()) + ": " + parMap[dt.paramID()] + ": " + ori;
    chList.push_back(ch);
    newCorr.push_back(dt.original());
  }
  ApproveDialog* approveDialog = new ApproveDialog(chList);
  approveDialog->setWindowTitle(tr("%1 - Godkjenning av data").arg(QApplication::applicationName()));
  int res = approveDialog->exec();
  if ( res == QDialog::Accepted )
  for ( int irow = firstRow; irow <= lastRow; irow++) {
    QModelIndex index = dataModel->index(irow, column);
    dataModel->setAcceptedData(index, newCorr[irow-firstRow]);
  }
  return;
}

void HqcMainWindow::rejectTimeseriesOK() {
  QDateTime stime;
  QDateTime etime;
  QString parameter;
  int stationIndex;
  int parameterIndex;
  bool result = rjtsdlg->getResults(parameter,stime,etime,stationIndex);
  if ( !result ) return;
  std::list<kvalobs::kvParam>::const_iterator it=plist.begin();
  for(;it!=plist.end(); it++){
    if ( it->name().cStr() == parameter ){
      parameterIndex = it->paramID();
      break;
    }
  }
  WhichDataHelper whichData;
  long int stnr = stationIndex;
  miutil::miTime ft;
  miutil::miTime tt;
  ft.setTime(miutil::miString(stime.toString("yyyy-MM-dd hh:mm:ss").toStdString()));
  tt.setTime(miutil::miString(etime.toString("yyyy-MM-dd hh:mm:ss").toStdString()));
  whichData.addStation(stnr, ft, tt);
  checkTypeId(stnr);
  int firstRow = dataModel->dataRow(stnr, ft);
  int lastRow  = dataModel->dataRow(stnr, tt);
  int column   = dataModel->dataColumn(parameter);

  QString ch;
  vector<QString> chList;
  for ( int irow = firstRow; irow <= lastRow; irow++) {
    QModelIndex index = dataModel->index(irow, column);
    const kvData & dt = dataModel->getKvData_(index);
    if ( dt.corrected() < -32760 )
      continue;
    QString cr;
    cr = cr.setNum(dt.corrected(), 'f', 1);
    QString stnr;
    stnr = stnr.setNum(dt.stationID());
    ch = stnr + " " + QString(dt.obstime().isoTime().cStr()) + ": " + parMap[dt.paramID()] + ": " + cr;
    chList.push_back(ch);
  }
  DiscardDialog* discardDialog = new DiscardDialog(chList);
  discardDialog->setWindowTitle(tr("%1 - Forkasting av data").arg(QApplication::applicationName()));
  int res = discardDialog->exec();
  if ( res == QDialog::Accepted )
  for ( int irow = firstRow; irow <= lastRow; irow++) {
    QModelIndex index = dataModel->index(irow, column);
    dataModel->setDiscardedData(index, -32766);
  }
  return;
}

void HqcMainWindow::startKro() {
  system("firefox kro/cgi-bin/start.pl &");
}

void HqcMainWindow::closeEvent(QCloseEvent* event)
{
  writeSettings();
  QWidget::closeEvent(event);
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
  stnr = (*datalist)[index].stnr();
  obstime = (*datalist)[index].otime();
  showTypeId = (*datalist)[index].showTypeId();
  typeIdChanged = (*datalist)[index].typeIdChanged();
  int hour = obstime.hour();
  for ( int i = 0; i < NOPARAM; i++ ) {
    typeId[i] = (*datalist)[index].typeId(i);
    orig[i] = (*datalist)[index].orig(i);
    flag[i] = (*datalist)[index].flag(i);
    corr[i] = (*datalist)[index].corr(i);
    controlinfo[i] = (*datalist)[index].controlinfo(i).flagstring();
    std::cout << '<' << controlinfo[i].c_str() <<  '>' << std::endl;
    useinfo[i] = (*datalist)[index].useinfo(i).flagstring();
    cfailed[i] = (*datalist)[index].cfailed(i);
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

bool HqcMainWindow::timeFilter(int hour) {
  if ( clkdlg->clk[hour]->isChecked() )
    return TRUE;
  return FALSE;
}

bool HqcMainWindow::hqcTypeFilter(const int& typeId, int environment, int stnr) {
  if ( typeId == -1 || typeId == 501 ) return FALSE;
  //  if ( typeId == -1 ) return FALSE;
  if ( lstdlg->webReg->isChecked() || lstdlg->priReg->isChecked() ) return TRUE;
  int atypeId = typeId < 0 ? -typeId : typeId;
  if (  lstdlg->allType->isChecked() ) return TRUE;
  if ( environment == 1 && atypeId == 311 && lstdlg->afType->isChecked() ) return TRUE;
  if ( (environment == 8 && (atypeId == 3 || atypeId == 311 || atypeId == 412)) || (atypeId == 330 || atypeId == 342) && lstdlg->aaType->isChecked() ) return TRUE;
  if ( environment == 2 && atypeId == 3 && lstdlg->alType->isChecked() ) return TRUE;
  if ( environment == 12 && atypeId == 3 && lstdlg->avType->isChecked() ) return TRUE;
  if ( atypeId == 410 && lstdlg->aoType->isChecked() ) return TRUE;
  if ( environment == 7 && lstdlg->mvType->isChecked() ) return TRUE;
  if ( environment == 5 && lstdlg->mpType->isChecked() ) return TRUE;
  if ( environment == 4 && lstdlg->mmType->isChecked() ) return TRUE;
  if ( environment == 6 && lstdlg->msType->isChecked() ) return TRUE;
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
  // Midlertidig spesialtilfelle for Oppdal!!!!!!!!!!!!!!!
  //
  if ( stnr == 63705 ) {
    if ( typeId == -330 )
      return false;
  }
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
  //  if ( par == 173 && typeId < 0 ) return false;
  if ( typeId == -404 ) return false;
  if ( stnr == 4780 && typeId == 1 ) return true;
  if ( (stnr == 68860 ) && typeId == -4 ) return false;
  if ( typeId < 0 && !((stnr == 18700 || stnr == 50540 || stnr == 90450 || stnr == 99910) && par == 109) ) return true;
  for ( vector<currentType>::iterator it = currentTypeList.begin(); it != currentTypeList.end(); it++) {
    if ( stnr == it->stnr &&
	 abs(typeId) == it->cTypeId &&
	 sensor == it->cSensor &&
	 par == it->par &&
	 (it->fDate.undef() || otime.date() >= it->fDate) &&
	 (it->tDate.undef() || otime.date() <= it->tDate) ) {
      tpf = true;
      break;
    }
  }
  return tpf;
}

/**
 * Read the data table in the Kvalobs database.
 *
 * Results will eventually end up in datalist variable
*/
void HqcMainWindow::readFromData(const miutil::miTime& stime, 
				 const miutil::miTime& etime, 
				 listType lity) {
  BusyIndicator busy();

  bool result;
  QTime t;
  t.start();
  t.restart();
  WhichDataHelper whichData;
  for ( int i = 0; i < stList.size(); i++ ) {
    whichData.addStation(stList[i], stime, etime);
    checkTypeId(stList[i]);
  }
 
  t.restart();
  
  // Throw away old data list. It will still exist if any window need it.
  datalist = model::KvalobsDataListPtr(new model::KvalobsDataList);
  KvObsDataList ldlist;// = GetData::datalist;
  GetData dataReceiver(this);
  if(!KvApp::kvApp->getKvData(dataReceiver, whichData)){
    //cerr << "Finner ikke  datareceiver!!" << endl;
  }
  else {
    //cerr << "Datareceiver OK!" << endl;
  }
}


/*!
 Read the modeldata table in the kvalobs database
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
  t.restart();

  if(!KvApp::kvApp->getKvModelData(mdlist, whichData))
    cerr << "Can't connect to modeldata table!" << endl;
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
}

/*!
 Read station info from the stinfosys database
*/
bool HqcMainWindow::readFromStInfoSys() {

  webs << 4780 << 7010 << 10380 << 16610 << 17150 << 18700 << 23420 << 24890 << 27500
       << 31620 << 36200 << 39100 << 42160 << 44560 << 47300 << 50500 << 50540 << 54120
       << 59800 << 60500 << 69100 << 71550 << 75410 << 75550 << 80610 << 82290 << 85380
       << 87110 << 89350 << 90450 << 93140 << 93700 << 94500 << 96400 << 98550 << 99370 
       << 99710 << 99720 << 99840 << 99910 << 99950;

  pri1s << 4780 << 10380 << 18500 << 18700 << 24890 << 27500 << 36200 << 39040 << 44560 
	<< 50540 << 54120 << 60500 << 69100 << 70850 << 72800 << 82290 << 90450 << 93700 
	<< 97251 << 98550 << 99710 << 99720 << 99840 << 99910 << 99950;

  pri2s << 3190 << 17150 << 28380 << 35860 << 60990 << 63420 << 68860 << 68863 << 93140 << 94500 << 95350;

  pri3s << 180 << 700 << 1130 << 2540 << 4440 << 4460 << 6020 << 7010 << 8140 << 9580 << 11500 << 12320 	
	<< 12550 << 12680 << 13160 << 13420 << 13670 << 15730 << 16610 << 16740 << 17000 << 18950 
	<< 19710 << 20301 << 21680 << 23160 << 23420 << 23500 << 23550 << 25110 << 25830 << 26900 
	<< 26990 << 27450 << 27470 << 28800 << 29720 << 30420 << 30650 << 31620 << 32060 << 33890 
	<< 34130 << 36560 << 37230 << 38140 << 39100 << 39690 << 40880 << 41110 << 41670 << 41770 
	<< 42160 << 42920 << 43010 << 44080 << 44610 << 45870 << 46510 << 46610 << 46910 << 47260 
	<< 47300 << 48120 << 48330 << 49800 << 50070 << 50300 << 50500 << 51530 << 51800 << 52290 
	<< 52535 << 52860 << 53101 << 55290 << 55700 << 55820 << 57420 << 57710 << 57770 << 58070 
	<< 58900 << 59110 << 59610 << 59680 << 59800 << 61180 << 61770 << 62270 << 62480 << 63705 
	<< 64330 << 64550 << 65310 << 65940 << 66730 << 68340 << 69150 << 69380 << 70150 << 71000 
	<< 71850 << 71990 << 72060 << 72580 << 73500 << 75220 << 75410 << 75550 << 76330 << 76450 
	<< 76530 << 76750 << 77230 << 77550 << 78800 << 79600 << 80101 << 80102 << 80610 << 80700 
	<< 81680 << 82410 << 83550 << 85380 << 85450 << 85891 << 85892 << 86500 << 86740 << 87110 
	<< 87640 << 88200 << 88690 << 89350 << 90490 << 90800 << 91380 << 91760 << 92350 << 93301 
	<< 93900 << 94280 << 94680 << 96310 << 96400 << 96800 << 97350 << 98090 << 98400 << 98790 
	<< 99370 << 99760 << 99765;

  coast << 17000 << 17280 << 17290 << 27080 << 27150 << 27230 << 27240 << 27270 << 27410 << 27490 
	<< 27500 << 29950 << 30000 << 34080 << 34120 << 34130 << 35850 << 35860 << 36150 << 36200 
	<< 38110 << 39040 << 39100 << 39170 << 39350 << 41090 << 41100 << 41110 << 41750 << 41760
	<< 41770 << 41772 << 42160 << 43270 << 43340 << 43350 << 43900 << 44080 << 44320 << 44560 
	<< 44600 << 44610 << 44630 << 44640 << 45870 << 45880 << 45900 << 47190 << 47200 << 47210 
	<< 47260 << 47300 << 48120 << 48300 << 48330 << 50460 << 50500 << 50540 << 50541 << 50542 
	<< 50543 << 50544 << 50560 << 50700 << 52530 << 52535 << 52800 << 52860 << 56420 << 56440 
	<< 57710 << 57720 << 57740 << 57750 << 57760 << 57770 << 59100 << 59110 << 59580 << 59610 
	<< 59800 << 59810 << 60830 << 60945 << 60950 << 60990 << 61040 << 61060 << 61150 << 61170 
	<< 61180 << 62270 << 62300 << 62310 << 62480 << 62490 << 62500 << 62640 << 62650 << 62660 
	<< 64250 << 64260 << 64330 << 65300 << 65310 << 65340 << 65370 << 65450 << 65720 << 65940 
	<< 65950 << 71540 << 71550 << 71560 << 71650 << 71780 << 71850 << 71980 << 71990 << 75220 
	<< 75300 << 75350 << 75410 << 75550 << 75600 << 75700 << 76300 << 76310 << 76320 << 76330 
	<< 76450 << 76500 << 76530 << 76600 << 76750 << 76810 << 76820 << 76850 << 76900 << 76920 
	<< 76923 << 76925 << 76926 << 76928 << 76930 << 76931 << 76932 << 76933 << 76940 << 76941 
	<< 76942 << 76943 << 76944 << 76945 << 76946 << 76947 << 76948 << 76949 << 76950 << 76951 
	<< 76970 << 76971 << 76974 << 76978 << 76980 << 76985 << 77002 << 77005 << 77006 << 77020 
	<< 77021 << 77022 << 77023 << 77024 << 80100 << 80101 << 80102 << 80300 << 80610 << 80740 
	<< 80900 << 80950 << 82240 << 82250 << 82260 << 82270 << 82280 << 82290 << 82400 << 82410 
	<< 83120 << 83550 << 83700 << 83710 << 85030 << 85040 << 85230 << 85240 << 85380 << 85450 
	<< 85460 << 85560 << 85700 << 85780 << 85840 << 85890 << 85891 << 85900 << 85910 << 85950 
	<< 86150 << 86200 << 86230 << 86260 << 86500 << 86520 << 86600 << 86740 << 86750 << 86760 
	<< 86780 << 87100 << 87110 << 87111 << 87120 << 87350 << 87400 << 87410 << 87420 << 87640 
	<< 88580 << 88680 << 88690 << 90280 << 90400 << 90440 << 90450 << 90490 << 90500 << 90700 
	<< 90720 << 90721 << 90760 << 90800 << 90900 << 91190 << 92700 << 92750 << 93000 << 94250 
	<< 94260 << 94280 << 94350 << 94450 << 94500 << 94600 << 94680 << 94700 << 95800 << 96300 
	<< 96310 << 96400 << 96550 << 98090 << 98360 << 98400 << 98550 << 98580 << 98700 << 98790 
	<< 98800 << 99710 << 99720 << 99735 << 99737 << 99754 << 99765 << 99790 << 99821 << 99950 
	<< 99970;

  foreignId << 99986 << 99990 << 202000 << 203600 << 208000 << 210400 << 211000 
	    << 220600 << 222200 << 230800 << 241800 << 250000 << 280700;

  foreignName << " " << " " << "NORDLAND" << "NORDLAND" << "FINNMARK" << "NORDLAND" << "NORDLAND"
	      << "NORD-TRØNDELAG" << "NORD-TRØNDELAG" << "SØR-TRØNDELAG" << "ØSTFOLD"
	      << "ØSTFOLD" << "FINNMARK";

  if (!connect2stinfosys()) {
    cerr << "Cannot connect to stinfosys" << endl;
    return false;
  }
  QSqlQuery query1;
  if ( !query1.exec("select distinct x.stationid, y.name from station x, municip y where (x.stationid<=99999 and (x.municipid/100=y.municipid or (x.municipid<100 and x.municipid=y.municipid) or (x.municipid = 2800 and y.municipid = 2800))) order by x.stationid") ) {
    cerr << "Query1 failed" << endl;
    return false;
  }
  list<countyInfo> cList;
  while ( query1.next() ) {
    int stationid1 = query1.value(0).toInt();
    QString name1 = query1.value(1).toString();
    if ( name1 == "SVALBARD" || name1 == "JAN MAYEN" )
      name1 = "ISHAVET";
    countyInfo cInfo;
    cInfo.stnr = stationid1;
    cInfo.county = name1;
    cList.push_back(cInfo);
  }
  QSqlQuery query2;
  if ( !query2.exec("select distinct x.stationid, y.name from station x, municip y where (x.stationid<=99999 and (x.municipid=y.municipid or (x.municipid=2300 and x.municipid/100=y.municipid))) order by x.stationid") ) {
      cerr << "Query2 failed" << endl;
      return false;
  }
  
  list<countyInfo>::iterator it = cList.begin();
  if ( query1.size() == query2.size() ) {
    
    while ( query2.next() ) {
      int stationid2 = query2.value(0).toInt();
      if ( stationid2 == it->stnr ) {
	QString name2 = query2.value(1).toString();
	it->municip = name2;
	it++; 
      }
      else {
	cout << "Feil i stinfo" << endl;
	return false;
      }
    }
    
    for (list<countyInfo>::iterator it = cList.begin(); it != cList.end(); it++ ) {
      if ( webs.indexOf(it->stnr) >= 0 )
	it->web = "WEB";
      if ( pri1s.indexOf(it->stnr) >= 0 )
	it->pri = "PRI1";
      if ( pri2s.indexOf(it->stnr) >= 0 )
	it->pri = "PRI2";
      if ( pri3s.indexOf(it->stnr) >= 0 )
	it->pri = "PRI3";
      if ( coast.indexOf(it->stnr) >= 0 )
	it->ki = "K";
      else
	it->ki = "I";

      int snr = 0;
      QString strSnr("    ");
    }
    //Some additional stations
    for ( int i = 0; i < foreignId.size(); i++ ) {
      countyInfo cInfo;
      cInfo.stnr = foreignId[i];
      cInfo.county = foreignName[i];
      cInfo.ki = "I";
      cList.push_back(cInfo);
    }
    QString path = QString(getenv("HOME"));
    QString statFile = path + "/.config/hqc_stations";
    ofstream outf(statFile);
    if ( !outf.is_open() ) {
      cerr << "FILFEIL" << endl;
      exit(1);
    }
    QFile stations(statFile);
    if ( !stations.open(QIODevice::WriteOnly) ) {
      cerr << "FEIL I FIL" << endl;
      //      exit(1);
    } 
    for (list<countyInfo>::iterator it = cList.begin(); it != cList.end(); it++ ) {
      int snr = 0;
      QString strSnr("    ");

      std::list<kvalobs::kvStation>::const_iterator sit=slist.begin();
      bool foundStation = FALSE;

      for(;sit!=slist.end(); sit++){
	if ( sit->stationID() == it->stnr ) {
	  foundStation = TRUE;
	  if ( sit->wmonr() > 0 )
	    strSnr = strSnr.setNum(sit->wmonr());
	  break;
	}
      }
      if ( foundStation ) {
	QString strStnr;
	QString strHoh;
	QString strEnv;
	strEnv = strEnv.setNum(sit->environmentid());
	listStatName.append(sit->name().cStr());
	listStatNum.append(strStnr.setNum(sit->stationID()));
	listStatHoh.append(strHoh.setNum(sit->height()));
	listStatType.append(strEnv);
	listStatFylke.append(it->county);
	listStatKommune.append(it->municip);
	listStatWeb.append(it->web);
	listStatPri.append(it->pri);
	listStatCoast.append(it->ki);
      }
      
      outf << setw(7) << right << it->stnr << " " 
	   << setw(31) << left << (it->county).toStdString() 
	   << setw(25) << (it->municip).toStdString() 
	   << setw(3) << (it->web).toStdString() 
	   << setw(4) << (it->pri).toStdString() 
	   << setw(1) << (it->ki).toStdString()
	   <<  endl;
      
    }
    outf.close();
    cout << "Stationliste hentet fra stinfosys" << endl;
  }
  else {
    cout << "Forskjellig lengde" << endl;
    return false;
  } 
  return true;
}

/*!
 Read the station table in the kvalobs database
*/
void HqcMainWindow::readFromStation() {
  if (!KvApp::kvApp->getKvStations(slist)) {
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
      crT.cSensor = sensor;
      crT.cTypeId = typeId;
      currentTypeList.push_back(crT);
    }
  }
}

/*!
 Read the station file, this must be done after the station table in the database is read
*/

void HqcMainWindow::readFromStationFile(int statCheck) {
  QString path = QString(getenv("HOME"));
  if ( path.isEmpty() ) {
    cerr << "Intet environment" << endl;
    exit(1);
  }
  QString stationFile = path + "/.config/hqc_stations";
  QFile stations(stationFile);
  stations.open(QIODevice::ReadOnly);
  QTextStream stationStream(&stations);
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
      listStatCoast.append(statLine.mid(71,1).stripWhiteSpace());
    }
    prevStnr = stnr;
  }
}

/*!
 Read the param table in the kvalobs database
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
  QTextStream paramStream(&paramOrder);
  int i = 0;

  parameterGroups.clear();
  QString group;
  while ( not paramStream.atEnd() ) {
      QString data;
      paramStream >> data;
      data.stripWhiteSpace();
      if ( not data.isEmpty() ) {
        bool ok;
        int paramid = data.toInt(& ok);
        if ( not ok )
          group = data;
        else {
            QString name = "<No group>";
            std::map<QString, QString>::const_iterator find = configNameToUserName.find(group);
            if ( find != configNameToUserName.end() )
              name = find->second;
            parameterGroups[name].push_back(paramid);
        }
      }
  }

  bool result;

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
      std::string cName = it->name();
      name = QString(cName.c_str());
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

//  ws->tileSubWindows();

  // primitive horizontal tiling
  QList<QMdiSubWindow *> windows = ws->subWindowList();
  if ( windows.empty() )
    return;

  int y = 0;
  if ( windows.count() == 1 ) {
    QWidget *window = windows.at(0);
    window->showMaximized();
    //window->parentWidget()->setGeometry( 0, y, ws->width(), ws->height() );
    return;
  }
  else {
    int height[] = {0, 28 + ws->height() / 2, (ws->height() / 2) - 28} ;

    for ( int i = windows.count() -2; i < windows.count(); ++ i ) {
    //for ( int i = int(windows.count()) - 1; i > int(windows.count()) -2; --i ) {
      QWidget *window = windows.at(i);

      qDebug() << window->caption();

      if ( window->windowState() == Qt::WindowMaximized ) {
	// prevent flicker
	window->hide();
      }
      window->showNormal();
      int preferredHeight = window->minimumHeight()+window->parentWidget()->baseSize().height();
      int actHeight = QMAX(height[i -int(windows.count()) + 3], preferredHeight);

      window->setGeometry( 0, y, ws->width(), actHeight );
      y += actHeight;
    }
  }
}

void HqcMainWindow::closeWindow()
{
  firstObs=true;
  cerr << "HqcMainWindow::closeWindow()\n";

  ws->closeActiveSubWindow();

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
			"Knut Johansen, IT\n ");
}

/*
namespace
{
class FunctionLogger
{
  const std::string name_;

  static int indent;

  void log_(const std::string & msg) const
  {
    std::ostringstream data;
    for ( int i = 0; i < indent; ++ i )
      data << "--------";
    data << "> " << msg.c_str();
    std::string out = data.str();
    qDebug() << out.c_str();
  }

public:
  FunctionLogger(const char * name) : name_(name)
  {
    ++ indent;
    log_("Entering " + name_);
  }
  ~FunctionLogger()
  {
    log_("Leaving  " + name_);
    -- indent;
  }
};
int FunctionLogger::indent = 3;
}
#define LOG_FUNCTION() FunctionLogger INTERNAL_function_logger(__func__)
*/

void HqcMainWindow::initDiana()
{
  LOG_FUNCTION();
  dianaconnected= false;

  std::string type1 = "Diana";
  std::string type2 = "hqc";

  if(pluginB->clientTypeExist(type1)){
    dianaconnected= true;
  }
  sendTimes();
  sendAnalysisMessage();
  return;
}

// send one image to diana (with name)
void HqcMainWindow::sendImage(const miutil::miString name, const QImage& image)
{
  LOG_FUNCTION();
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
    cerr << "HQC sender melding : " << m.content() << endl;
  pluginB->sendMessage(m);
}

// called when client-list changes
void HqcMainWindow::processConnect()
{
  LOG_FUNCTION();
  initDiana();
}

void HqcMainWindow::cleanConnection()
{
  dianaconnected = false;
  firstObs = true;
  cout << "< DISCONNECTING >" << endl;
}

void HqcMainWindow::sendTimes(){
  LOG_FUNCTION();

  //send times
  miMessage m;
  m.command= qmstrings::settime;
  m.commondesc = "datatype";
  m.common = "obs";
  m.description= "time";
  int n=datalist->size();
  for(int i=0;i<n;i++){
    m.data.push_back((*datalist)[i].otime().isoTime());
  }
    cerr << "HQC sender melding : " << m.content() << endl;
  pluginB->sendMessage(m);
  
}

// processes incoming miMessages
void HqcMainWindow::processLetter(miMessage& letter)
{
  LOG_FUNCTION();
  if(letter.command == qmstrings::newclient) {
    firstObs = true;
    processConnect(); 
    hqcFrom = letter.to;
    hqcTo = letter.from;
  }
  else if (letter.command == "station" ) {
    const char* ccmn = letter.common.c_str();
    QString cmn = QString(ccmn);
    emit statTimeReceived(cmn);
  }
  else if(letter.command == qmstrings::timechanged){
      const char* ccmn = letter.common.c_str();
      QString cmn = QString(ccmn);
      //cerr << "Innkommende melding: statTimeReceived is emitted."  << endl;
      emit statTimeReceived(cmn);

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
  LOG_FUNCTION();
  
  //show analysis
  miMessage letter;
  letter.command=qmstrings::apply_quickmenu;
  letter.data.push_back("Hqc");
  letter.data.push_back("Bakkeanalyse");
    cerr << "HQC sender melding : " << letter.content() << endl;
  pluginB->sendMessage(letter);
  
  dianaTime.setTime(miutil::miString("2000-01-01 00:00:00"));
  return true;
}

void HqcMainWindow::sendStation(int stnr)
{
  LOG_FUNCTION();

  miMessage pLetter;
  pLetter.command = qmstrings::station;
  miutil::miString stationstr(stnr);
  pLetter.common = stationstr;
    cerr << "HQC sender melding : " << pLetter.content() << endl;
  pluginB->sendMessage(pLetter);
  
}

void HqcMainWindow::aboutQt()
{
    QMessageBox::aboutQt( this, "Qt is a C++ toolkit for application development. " );
}

void HqcMainWindow::sendObservations(const miutil::miTime & time, bool sendtime)
{
  LOG_FUNCTION();

  //no data -> return
  if(selPar.count() == 0) return;

  //just sent 
  if(dianaTime == time)
    return;

  dianaTime = time;

  if(sendtime){
    miMessage tLetter;
    tLetter.command = qmstrings::settime;
    tLetter.commondesc = "time";
    tLetter.common = time.isoTime();
    cerr << "HQC sender melding : " << tLetter.content() << endl;
    pluginB->sendMessage(tLetter);
  }
  miMessage pLetter;
  if ( firstObs ) { 
    pLetter.command = qmstrings::init_HQC_params;
  }
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

  int prStnr = 0;

  for ( int i = 0; i < datalist->size(); i++) { // fill data
    if ( (*datalist)[i].otime() == time || (firstObs && (*datalist)[i].stnr() != prStnr )){
      double lat,lon,h;
      miutil::miString str((*datalist)[i].stnr());
      QString name;
      int env;
      int snr;
      //      int typeId = (*datalist)[i].typeId;
      int typeId = (*datalist)[i].showTypeId();
      findStationInfo((*datalist)[i].stnr(),name,lat,lon,h,snr,env);
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
      double aa = (*datalist)[i].corr(1);
      for(int j=0; j<parIndex.size();j++){
	double corr = (*datalist)[i].corr(parIndex[j]);
	if ( parModel[j] ) {
	  for ( int k = 0; k < modeldatalist.size(); k++ ) {
	    if ( modeldatalist[k].stnr == (*datalist)[i].stnr() &&
		 modeldatalist[k].otime == (*datalist)[i].otime() ) {
	      corr = modeldatalist[k].orig[parIndex[j]];
	      break;
	    }
	  }
	}
	if ( parDiff[j] ) {
	  for ( int k = 0; k < modeldatalist.size(); k++ ) {
	    if ( modeldatalist[k].stnr == (*datalist)[i].stnr() &&
		 modeldatalist[k].otime == (*datalist)[i].otime() ) {
	      corr = corr - modeldatalist[k].orig[parIndex[j]];
	      break;
	    }
	  }
	}
	if ( parProp[j] ) {
	  for ( int k = 0; k < modeldatalist.size(); k++ ) {
	    if ( modeldatalist[k].stnr == (*datalist)[i].stnr() &&
		 modeldatalist[k].otime == (*datalist)[i].otime() ) {
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
	  int flag = (*datalist)[i].flag(parIndex[j]);
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
    prStnr = (*datalist)[i].stnr();
  }
  //send letters
  if(synopData.size()){
    firstObs = false;
    pLetter.description = synopDescription;
    pLetter.common = time.isoTime() + ",synop";
    pLetter.data = synopData;
    //TEST
    cerr << "HQC sender melding : " << pLetter.content() << endl;
    //TEST
    pluginB->sendMessage(pLetter);
  }
  /*
  if( !synopData.size() ) {
    miMessage okLetter;
    okLetter.command = "menuok";
    okLetter.from = hqcFrom;
    okLetter.to = hqcTo;
    pluginB->sendMessage(okLetter);
  }
  */
}



void HqcMainWindow::sendSelectedParam(const QString & param)
{
  LOG_FUNCTION();

  miutil::miString diParam = dianaName(param.latin1());
  if(!diParam.exists()) {
      qDebug() << qPrintable(param) << ": No such diana parameter";
      return;
  }

  miMessage pLetter;
  pLetter.command = qmstrings::select_HQC_param;
  pLetter.commondesc = "diParam";
  pLetter.common = diParam;
    cerr << "HQC sender melding : " << pLetter.content() << endl;
  pluginB->sendMessage(pLetter);
}


void HqcMainWindow::updateParams(int station, 
    const miutil::miTime & time,
    const miutil::miString & param,
    const miutil::miString & value,
    const miutil::miString & flag)
{
  LOG_FUNCTION();

  //Update datalist
  int i=0;
  int n= datalist->size();
  while( i<n && ( (*datalist)[i].stnr() !=station || (*datalist)[i].otime() != time)) i++;
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
  double cdValue = dianaValue(parameterIndex, false, atof(value.cStr()),(*datalist)[i].corr(1));
  int    iflag  = atoi(flag.cStr());
  (*datalist)[i].set_corr(parameterIndex, dValue);
  //(*datalist)[i].set_flag(parameterIndex, iflag);
  miutil::miString value_flag = miutil::miString(cdValue) + ":" + flag;

  //update timeseries
//  TimeseriesOK();

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
    cerr << "HQC sender melding : " << pLetter.content() << endl;
  pluginB->sendMessage(pLetter);

  pLetter.common = time.isoTime() + ",synop";
  miutil::miString dianaParam = dianaName(param);
  if(dianaParam.exists()){
    pLetter.description = "id," + dianaParam;
    cerr << "HQC sender melding : " << pLetter.content() << endl;
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


// Help function to translate from kvalobs parameter names to diana parameter names 
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

void HqcMainWindow::updateSaveFunction( QMdiSubWindow * w )
{
  if ( ! w )
    return;

  ErrorList *win = dynamic_cast<ErrorList*>(w->widget());
  bool enabled = win;
  saveAction->setEnabled(enabled);
}

bool HqcMainWindow::isAlreadyStored(miutil::miTime otime, int stnr) {
  for ( int i = 0; i < datalist->size(); i++) {
    if ( (*datalist)[i].otime() == otime && (*datalist)[i].stnr() == stnr )
      return true;
  }
  return false;
}
 
int HqcMainWindow::findTypeId(int typ, int pos, int par, miutil::miTime oTime)
{
  int tpId;
  tpId = typ;
  for(CIObsPgmList obit=obsPgmList.end();obit!=obsPgmList.begin(); obit--){
    if ( obit->stationID() == pos && obit->paramID() == par && obit->fromtime() < oTime && obit->totime() > oTime ) {
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
      else {
	aggPar = 0;
	aggTyp = 0;
	aggStat = 0;
	aggTime = "1800-01-01 00:00:00" ;
      }

      int stnr = dit->stationID();
      miutil::miTime otime = (dit->obstime());
      miutil::miTime tbtime = (dit->tbtime());
      int hour = otime.hour();
      int typeId = dit->typeID();
      int sensor = dit->sensor();
      if ( (otime == protime && stnr == prstnr && dit->paramID() == prParam && typeId == prtypeId && sensor == prSensor 
	    && lstdlg->priTypes->isChecked()) || (!correctTypeId && !lstdlg->priTypes->isChecked()) ) {
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
	tdl.set_corr(dit->paramID(), dit->corrected());
	tdl.set_sensor(dit->paramID(), dit->sensor());
	tdl.set_level(dit->paramID(), dit->level());
	tdl.set_controlinfo(dit->paramID(), dit->controlinfo());
      	tdl.set_useinfo(dit->paramID(), dit->useinfo());
	tdl.set_cfailed(dit->paramID(), dit->cfailed());
	if ( typeId != 501 )
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
      tdl.set_latitude(lat);
      tdl.set_longitude(lon);
      tdl.set_altitude(hoh);
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
	    ((otime != protime || ( otime == protime && stnr != prstnr)))) ||
	   (lstdlg->allTypes->isChecked() && typeId != prtypeId) ) {
	datalist->push_back(tdl);
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

void HqcMainWindow::writeSettings()
{
  QList<Param> params;

  QSettings settings("Meteorologisk Institutt", "Hqc");
  settings.setValue("geometry", saveGeometry());

  settings.beginWriteArray("t");
  for ( int hour = 0; hour < 24; hour++ ) {
    settings.setArrayIndex(hour);
    settings.setValue("t", clkdlg->clk[hour]->isChecked());
  }
  settings.endArray();

  int st = 0;
  settings.beginWriteArray("s");
  settings.setArrayIndex(st++);
  settings.setValue("s",lstdlg->aaType->isChecked());
  settings.setArrayIndex(st++);
  settings.setValue("s",lstdlg->afType->isChecked());
  settings.setArrayIndex(st++);
  settings.setValue("s",lstdlg->alType->isChecked());
  settings.setArrayIndex(st++);
  settings.setValue("s",lstdlg->avType->isChecked());
  settings.setArrayIndex(st++);
  settings.setValue("s",lstdlg->aoType->isChecked());
  settings.setArrayIndex(st++);
  settings.setValue("s",lstdlg->aeType->isChecked());
  settings.setArrayIndex(st++);
  settings.setValue("s",lstdlg->mvType->isChecked());
  settings.setArrayIndex(st++);
  settings.setValue("s",lstdlg->mpType->isChecked());
  settings.setArrayIndex(st++);
  settings.setValue("s",lstdlg->mmType->isChecked());
  settings.setArrayIndex(st++);
  settings.setValue("s",lstdlg->msType->isChecked());
  settings.setArrayIndex(st++);
  settings.setValue("s",lstdlg->fmType->isChecked());
  settings.setArrayIndex(st++);
  settings.setValue("s",lstdlg->nsType->isChecked());
  settings.setArrayIndex(st++);
  settings.setValue("s",lstdlg->ndType->isChecked());
  settings.setArrayIndex(st++);
  settings.setValue("s",lstdlg->noType->isChecked());
  settings.setArrayIndex(st++);
  settings.setValue("s",lstdlg->piType->isChecked());
  settings.setArrayIndex(st++);
  settings.setValue("s",lstdlg->ptType->isChecked());
  settings.setArrayIndex(st++);
  settings.setValue("s",lstdlg->vsType->isChecked());
  settings.setArrayIndex(st++);
  settings.setValue("s",lstdlg->vkType->isChecked());
  settings.setArrayIndex(st++);
  settings.setValue("s",lstdlg->vmType->isChecked());
  settings.setArrayIndex(st++);
  settings.setValue("s",lstdlg->allType->isChecked());
  settings.endArray();

  int fy = 0;
  settings.beginWriteArray("c");
  settings.setArrayIndex(fy++);
  settings.setValue("c",lstdlg->oslCoun->isChecked());
  settings.setArrayIndex(fy++);
  settings.setValue("c",lstdlg->akeCoun->isChecked());
  settings.setArrayIndex(fy++);
  settings.setValue("c",lstdlg->ostCoun->isChecked());
  settings.setArrayIndex(fy++);
  settings.setValue("c",lstdlg->hedCoun->isChecked());
  settings.setArrayIndex(fy++);
  settings.setValue("c",lstdlg->oppCoun->isChecked());
  settings.setArrayIndex(fy++);
  settings.setValue("c",lstdlg->busCoun->isChecked());
  settings.setArrayIndex(fy++);
  settings.setValue("c",lstdlg->vefCoun->isChecked());
  settings.setArrayIndex(fy++);
  settings.setValue("c",lstdlg->telCoun->isChecked());
  settings.setArrayIndex(fy++);
  settings.setValue("c",lstdlg->ausCoun->isChecked());
  settings.setArrayIndex(fy++);
  settings.setValue("c",lstdlg->veaCoun->isChecked());
  settings.setArrayIndex(fy++);
  settings.setValue("c",lstdlg->rogCoun->isChecked());
  settings.setArrayIndex(fy++);
  settings.setValue("c",lstdlg->horCoun->isChecked());
  settings.setArrayIndex(fy++);
  settings.setValue("c",lstdlg->sogCoun->isChecked());
  settings.setArrayIndex(fy++);
  settings.setValue("c",lstdlg->morCoun->isChecked());
  settings.setArrayIndex(fy++);
  settings.setValue("c",lstdlg->sorCoun->isChecked());
  settings.setArrayIndex(fy++);
  settings.setValue("c",lstdlg->ntrCoun->isChecked());
  settings.setArrayIndex(fy++);
  settings.setValue("c",lstdlg->norCoun->isChecked());
  settings.setArrayIndex(fy++);
  settings.setValue("c",lstdlg->troCoun->isChecked());
  settings.setArrayIndex(fy++);
  settings.setValue("c",lstdlg->finCoun->isChecked());
  settings.setArrayIndex(fy++);
  settings.setValue("c",lstdlg->svaCoun->isChecked());
  settings.setArrayIndex(fy++);
  settings.setValue("c",lstdlg->allCoun->isChecked());
  settings.endArray();

  settings.setValue("weather", wElement);
  settings.beginWriteArray("p");
  for ( int jj = 0; jj < parFind; jj++ ) {
    settings.setArrayIndex(jj);
    qDebug() << qPrintable(pardlg->plb->item(jj)->text()) 
	     << pardlg->plb->item(jj)->isSelected() << ": (" 
	     << pardlg->markPar->isChecked() << "" 
	     << pardlg->noMarkPar->isChecked() << "" 
	     << pardlg->allPar->isChecked() << ")";
    settings.setValue("item", pardlg->plb->item(jj)->isSelected());
    settings.setValue("text", pardlg->plb->item(jj)->text());
    settings.setValue("mark", pardlg->markPar->isChecked());
    settings.setValue("noMark", pardlg->noMarkPar->isChecked());
    settings.setValue("all", pardlg->allPar->isChecked());
  }
  settings.endArray();
}

void HqcMainWindow::readSettings()
{
  QList<Param> params;

  QSettings settings("Meteorologisk Institutt", "Hqc");
  if ( !restoreGeometry(settings.value("geometry").toByteArray()) )
    cout << "CANNOT RESTORE GEOMETRY!!!!" << endl;
  
  bool times[24];
  settings.beginReadArray("t");
  for ( int hour = 0; hour < 24; hour++ ) {
    settings.setArrayIndex(hour);
    times[hour] = settings.value("t", true).toBool();
    clkdlg->clk[hour]->setChecked(times[hour]);
  }
  settings.endArray();
  
  wElement = settings.value("weather","").toString();
  parFind = settings.beginReadArray("p");
  for ( int jj = 0; jj < parFind; jj++ ) {
    settings.setArrayIndex(jj);
    Param param;
    param.item   = settings.value("item",true).toBool();
    QListWidgetItem* it = new QListWidgetItem(pardlg->plb,jj);
    it->setSelected(param.item);
    param.text   = settings.value("text","").toString();
    it->setText(param.text);
    param.mark   = settings.value("mark",true).toBool();
    pardlg->markPar->setChecked(param.mark);
    param.noMark = settings.value("noMark",true).toBool();
    pardlg->noMarkPar->setChecked(param.noMark);
    param.all    = settings.value("all",true).toBool();
    pardlg->allPar->setChecked(param.all);
    params.append(param);
  }
  settings.endArray();

  bool types[20];
  int tp = 0;
  settings.beginReadArray("s");
  settings.setArrayIndex(tp++);
  types[tp] = settings.value("s",true).toBool();
  lstdlg->aaType->setChecked(types[tp]);
  settings.setArrayIndex(tp++);
  types[tp] = settings.value("s",true).toBool();
  lstdlg->afType->setChecked(types[tp]);
  settings.setArrayIndex(tp++);
  types[tp] = settings.value("s",true).toBool();
  lstdlg->alType->setChecked(types[tp]);
  settings.setArrayIndex(tp++);
  types[tp] = settings.value("s",true).toBool();
  lstdlg->avType->setChecked(types[tp]);
  settings.setArrayIndex(tp++);
  types[tp] = settings.value("s",true).toBool();
  lstdlg->aoType->setChecked(types[tp]);
  settings.setArrayIndex(tp++);
  types[tp] = settings.value("s",true).toBool();
  lstdlg->aeType->setChecked(types[tp]);
  settings.setArrayIndex(tp++);
  types[tp] = settings.value("s",true).toBool();
  lstdlg->mvType->setChecked(types[tp]);
  settings.setArrayIndex(tp++);
  types[tp] = settings.value("s",true).toBool();
  lstdlg->mpType->setChecked(types[tp]);
  settings.setArrayIndex(tp++);
  types[tp] = settings.value("s",true).toBool();
  lstdlg->mmType->setChecked(types[tp]);
  settings.setArrayIndex(tp++);
  types[tp] = settings.value("s",true).toBool();
  lstdlg->msType->setChecked(types[tp]);
  settings.setArrayIndex(tp++);
  types[tp] = settings.value("s",true).toBool();
  lstdlg->fmType->setChecked(types[tp]);
  settings.setArrayIndex(tp++);
  types[tp] = settings.value("s",true).toBool();
  lstdlg->nsType->setChecked(types[tp]);
  settings.setArrayIndex(tp++);
  types[tp] = settings.value("s",true).toBool();
  lstdlg->ndType->setChecked(types[tp]);
  settings.setArrayIndex(tp++);
  types[tp] = settings.value("s",true).toBool();
  lstdlg->noType->setChecked(types[tp]);
  settings.setArrayIndex(tp++);
  types[tp] = settings.value("s",true).toBool();
  lstdlg->piType->setChecked(types[tp]);
  settings.setArrayIndex(tp++);
  types[tp] = settings.value("s",true).toBool();
  lstdlg->ptType->setChecked(types[tp]);
  settings.setArrayIndex(tp++);
  types[tp] = settings.value("s",true).toBool();
  lstdlg->vsType->setChecked(types[tp]);
  settings.setArrayIndex(tp++);
  types[tp] = settings.value("s",true).toBool();
  lstdlg->vkType->setChecked(types[tp]);
  settings.setArrayIndex(tp++);
  types[tp] = settings.value("s",true).toBool();
  lstdlg->vmType->setChecked(types[tp]);
  settings.setArrayIndex(tp++);
  types[tp] = settings.value("s",true).toBool();
  lstdlg->allType->setChecked(types[tp]);
  settings.endArray();

  bool county[21];
  int fy = 0;
  settings.beginReadArray("c");
  settings.setArrayIndex(fy++);
  county[fy] = settings.value("c",true).toBool();
  lstdlg->oslCoun->setChecked(county[fy]);
  settings.setArrayIndex(fy++);
  county[fy] = settings.value("c",true).toBool();
  lstdlg->akeCoun->setChecked(county[fy]);
  settings.setArrayIndex(fy++);
  county[fy] = settings.value("c",true).toBool();
  lstdlg->ostCoun->setChecked(county[fy]);
  settings.setArrayIndex(fy++);
  county[fy] = settings.value("c",true).toBool();
  lstdlg->hedCoun->setChecked(county[fy]);
  settings.setArrayIndex(fy++);
  county[fy] = settings.value("c",true).toBool();
  lstdlg->oppCoun->setChecked(county[fy]);
  settings.setArrayIndex(fy++);
  county[fy] = settings.value("c",true).toBool();
  lstdlg->busCoun->setChecked(county[fy]);
  settings.setArrayIndex(fy++);
  county[fy] = settings.value("c",true).toBool();
  lstdlg->vefCoun->setChecked(county[fy]);
  settings.setArrayIndex(fy++);
  county[fy] = settings.value("c",true).toBool();
  lstdlg->telCoun->setChecked(county[fy]);
  settings.setArrayIndex(fy++);
  county[fy] = settings.value("c",true).toBool();
  lstdlg->ausCoun->setChecked(county[fy]);
  settings.setArrayIndex(fy++);
  county[fy] = settings.value("c",true).toBool();
  lstdlg->veaCoun->setChecked(county[fy]);
  settings.setArrayIndex(fy++);
  county[fy] = settings.value("c",true).toBool();
  lstdlg->rogCoun->setChecked(county[fy]);
  settings.setArrayIndex(fy++);
  county[fy] = settings.value("c",true).toBool();
  lstdlg->horCoun->setChecked(county[fy]);
  settings.setArrayIndex(fy++);
  county[fy] = settings.value("c",true).toBool();
  lstdlg->sogCoun->setChecked(county[fy]);
  settings.setArrayIndex(fy++);
  county[fy] = settings.value("c",true).toBool();
  lstdlg->morCoun->setChecked(county[fy]);
  settings.setArrayIndex(fy++);
  county[fy] = settings.value("c",true).toBool();
  lstdlg->sorCoun->setChecked(county[fy]);
  settings.setArrayIndex(fy++);
  county[fy] = settings.value("c",true).toBool();
  lstdlg->ntrCoun->setChecked(county[fy]);
  settings.setArrayIndex(fy++);
  county[fy] = settings.value("c",true).toBool();
  lstdlg->norCoun->setChecked(county[fy]);
  settings.setArrayIndex(fy++);
  county[fy] = settings.value("c",true).toBool();
  lstdlg->troCoun->setChecked(county[fy]);
  settings.setArrayIndex(fy++);
  county[fy] = settings.value("c",true).toBool();
  lstdlg->finCoun->setChecked(county[fy]);
  settings.setArrayIndex(fy++);
  county[fy] = settings.value("c",true).toBool();
  lstdlg->svaCoun->setChecked(county[fy]);
  settings.setArrayIndex(fy++);
  county[fy] = settings.value("c",true).toBool();
  lstdlg->allCoun->setChecked(county[fy]);
  settings.endArray();
}

