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

#include "hqcmain.h"
#include "ui_mainwindow.h"

#include "accepttimeseriesdialog.h"
#include "approvedialog.h"
#include "BusyIndicator.h"
#include "ClockDialog.h"
#include "config.h"
#include "connect2stinfosys.h"
#include "dianashowdialog.h"
#include "discarddialog.h"
#include "errorlist.h"
#include "debug.hh"
#include "GetData.h"
#include "GetTextData.h"
#include "HintWidget.hh"
#include "identifyUser.h"
#include "hqc_paths.hh"
#include "hqc_utilities.hh"
#include "KvalobsDataModel.h"
#include "KvalobsDataView.h"
#include "ListDialog.h"
#include "MiDateTimeEdit.hh"
#include "mi_foreach.hh"
#include "parameterdialog.h"
#include "rejectdialog.h"
#include "rejecttable.h"
#include "rejecttimeseriesdialog.h"
#include "RRDialog.h"
#include "StationInformation.h"
#include "textdatadialog.h"
#include "TimeseriesDialog.h"
#include "timeutil.hh"
#include "weatherdialog.h"

#include <qTimeseries/TSPlot.h>
#include <qUtilities/ClientButton.h>
#include <qUtilities/miMessage.h>
#include <glText/glTextQtTexture.h>
#include <kvalobs/kvData.h>

#include <QtCore/qfile.h>
#include <QtCore/qsettings.h>
#include <QtCore/qtimer.h>
#include <QtCore/qurl.h>
#include <QtGui/QDesktopServices>
#include <QtGui/QMdiSubWindow>
#include <QtGui/QMessageBox>
#include <QtGui/QPixmap>
#include <QtGui/QPrinter>
#include <QtGui/QPrintDialog>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <qdebug.h>

#include <boost/algorithm/string.hpp>
#include <boost/assign.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include <algorithm>
#include <deque>
#include <functional>
#include <iomanip>
#include <stdexcept>

// declared in hqcdefs.h
const int modelParam[NOPARAMMODEL] =
    { 61, 81, 109, 110, 177, 178, 211, 262 };

namespace {

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

const int VERSION_CHECK_TIMEOUT = 60*60*1000; // milliseconds

} // anonymous namespace

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
  : QMainWindow( 0, tr("HQC"))
  , datalist(new model::KvalobsDataList)
  , reinserter( NULL )
  , dataModel(0)
  , firstObs(true)
  , sLevel(0)
  , listExist(false)
  , tsVisible(false)
  , ui(new Ui::HqcMainWindow)
  , dianaconnected(false)
  , mVersionCheckTimer(new QTimer(this))
  , mHints(new HintWidget(this))
{
    ui->setupUi(this);
    connect(ui->saveAction,  SIGNAL(triggered()), this, SIGNAL(saveData()));
    connect(ui->printAction, SIGNAL(triggered()), this, SIGNAL(printErrorList()));
    connect(ui->exitAction,  SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));
    connect(mVersionCheckTimer, SIGNAL(timeout()), this, SLOT(onVersionCheckTimeout()));

    mVersionCheckTimer->setSingleShot(true);

    QPixmap icon_listdlg( ::hqc::getPath(::hqc::IMAGEDIR) + "/table.png");
    ui->dataListAction->setIcon(icon_listdlg);

    QPixmap icon_ts( ::hqc::getPath(::hqc::IMAGEDIR) + "/kmplot.png");
    ui->timeSeriesAction->setIcon(icon_ts);

    pluginB = new ClientButton("hqc", "/usr/bin/coserver4", statusBar());
    pluginB->useLabel(true);
    pluginB->connectToServer();

#ifdef METLIBS_BEFORE_4_9_5
    connect(pluginB, SIGNAL(receivedMessage(miMessage&)), SLOT(processLetterOld(miMessage&)));
#else
    connect(pluginB, SIGNAL(receivedMessage(const miMessage&)), SLOT(processLetter(const miMessage&)));
#endif
    connect(pluginB, SIGNAL(addressListChanged()),
            SLOT(processConnect()));
    connect(pluginB, SIGNAL(connectionClosed()),
            SLOT(cleanConnection()));
    statusBar()->addPermanentWidget(pluginB, 0);


    // --- DEFINE DIALOGS --------------------------------------------
    const QString hqc_icon_path = ::hqc::getPath(::hqc::IMAGEDIR) + "/hqc.png";
    lstdlg = new ListDialog(this);
    lstdlg->setIcon( QPixmap(hqc_icon_path) );
    clkdlg = new ClockDialog(this);
    clkdlg->setIcon( QPixmap(hqc_icon_path) );
    pardlg = new ParameterDialog(this);
    pardlg->setIcon( QPixmap(hqc_icon_path) );
    dshdlg = new DianaShowDialog(this);
    dshdlg->setIcon( QPixmap(hqc_icon_path) );
    txtdlg = new TextDataDialog(slist, this);
    txtdlg->setIcon( QPixmap(hqc_icon_path) );
    rejdlg = new RejectDialog(this);
    rejdlg->setIcon( QPixmap(hqc_icon_path) );
    actsdlg = new AcceptTimeseriesDialog();
    actsdlg->hide();
    rjtsdlg = new RejectTimeseriesDialog();
    rjtsdlg->hide();
    
    // --- START -----------------------------------------------------
    pardlg->hide();
    rejdlg->hide();
    lstdlg->hide();

    connect( lstdlg, SIGNAL(ListApply()), SLOT(ListOK()));
    connect( lstdlg, SIGNAL(ListHide()), SLOT(listMenu()));

    clkdlg->hide();
    timeFilterChanged = false;
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
    
    connect(tsdlg, SIGNAL(TimeseriesApply()), SLOT(TimeseriesOK()));
    connect(tsdlg, SIGNAL(TimeseriesHide()), SLOT(timeseriesMenu()));
    
    connect(this, SIGNAL(newStationList(std::vector<QString>&)),
            tsdlg, SLOT(newStationList(std::vector<QString>&)));
    connect(this, SIGNAL(newParameterList(const QStringList&)),
            tsdlg, SLOT(newParameterList(const QStringList&)));
    
    connect(rjtsdlg, SIGNAL(tsRejectApply()), SLOT(rejectTimeseriesOK()));
    connect(rjtsdlg, SIGNAL(tsRejectHide()), SLOT(rejectTimeseries()));
    
    connect(actsdlg, SIGNAL(tsAcceptApply()), SLOT(acceptTimeseriesOK()));
    connect(actsdlg, SIGNAL(tsAcceptHide()), SLOT(acceptTimeseries()));
    
    connect(this,  SIGNAL(newStationList(std::vector<QString>&)),
            rjtsdlg, SLOT(newStationList(std::vector<QString>&)));
    connect(this,  SIGNAL(newParameterList(const QStringList&)),
            rjtsdlg, SLOT(newParameterList(const QStringList&)));
    
    connect(this,  SIGNAL(newStationList(std::vector<QString>&)),
            actsdlg, SLOT(newStationList(std::vector<QString>&)));
    connect(this,  SIGNAL(newParameterList(const QStringList&)),
            actsdlg, SLOT(newParameterList(const QStringList&)));
    
    connect(lstdlg, SIGNAL(fromTimeChanged(const QDateTime&)),
            tsdlg, SLOT(setFromTimeSlot(const QDateTime&)));
    
    connect(lstdlg, SIGNAL(toTimeChanged(const QDateTime&)),
            tsdlg, SLOT(setToTimeSlot(const QDateTime&)));
    
    // make the timeseries-plot-dialog
    tspdialog = new TSPlotDialog(this);
}

void HqcMainWindow::startup()
{
    listExist = false;
    firstObs = true;
    sLevel = 0;
    dianaconnected = false;
    tsVisible = false;
    dianaShowOK();

    // --- CHECK USER IDENTITY ----------------------------------------

    reinserter = Authentication::identifyUser(this, kvservice::KvApp::kvApp, "ldap-oslo.met.no", userName);
#if 0
    if( not reinserter ) {
        int mb = QMessageBox::information(this,
                                          tr("Autentisering"),
                                          tr("Du er ikke registrert som operatør!\n"
                                             "Du kan se dataliste, feillog og feilliste,\n"
                                             "men ikke gjøre endringer i Kvalobsdatabasen!"),
                                          tr("Fortsett"),
                                          tr("Avslutt"),
                                          "");
        if ( mb == 1 )
            throw runtime_error( "Not authenticated" );
    } else {
        cout << "Hei " << userName.toStdString() << ", du er registrert som godkjent operatør" << endl;
    }
#endif

    //-----------------------------------------------------------------

    readSettings();
    show();
    qApp->processEvents();
#if 1
    if( not reinserter ) {
        mHints->addHint(tr("<h1>Autentisering</h1>"
                           "Du er ikke registrert som operatør!<br/>"
                           "Du kan se dataliste, feillog og feilliste, "
                           "men ikke gjøre endringer i Kvalobsdatabasen!"));
    }
#endif

    // --- READ STATION INFO ----------------------------------------
    {
        BusyIndicator busy;
        readFromStation();
        qApp->processEvents();
        readFromObsPgm();
        qApp->processEvents();
        readFromParam();
        qApp->processEvents();
    }

    statusBar()->message( tr("Velkommen til kvhqc %1!").arg(PVERSION_FULL), 2000 );
    mVersionCheckTimer->start(VERSION_CHECK_TIMEOUT);
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

void HqcMainWindow::selectParameterGroup(const QString& group) {
    wElement = group;
    lity = daLi;
    firstObs = true;
    const std::vector<int> & parameters = parameterGroups[wElement];
    Q_ASSERT(not parameters.empty());
    pardlg->insertParametersInListBox(parameters, parMap);
    pardlg->showAll();
}

void HqcMainWindow::airPress() {
    selectParameterGroup("Lufttrykk");
}

void HqcMainWindow::temperature() {
    selectParameterGroup("Temperatur");
}

void HqcMainWindow::precipitation() {
    selectParameterGroup("Nedbør");
}

void HqcMainWindow::visuals() {
    selectParameterGroup("Visuell");
}

void HqcMainWindow::sea() {
    selectParameterGroup("Sjøgang");
}

void HqcMainWindow::synop() {
    selectParameterGroup("Synop");
}

void HqcMainWindow::climateStatistics() {
    selectParameterGroup("Klimastatistikk");
}

void HqcMainWindow::priority() {
    selectParameterGroup("Prioriterte parametere");
}

void HqcMainWindow::wind() {
    selectParameterGroup("Vind");
}

void HqcMainWindow::plu() {
    selectParameterGroup("Pluviometerkontroll");
}

void HqcMainWindow::all() {
    selectParameterGroup("Alt");
}

void HqcMainWindow::paramOK()
{
    LOG_SCOPE();
    if( listExist )
        ListOK();
}

void HqcMainWindow::ClkOK() {
  timeFilterChanged = true;
}

void HqcMainWindow::dianaShowOK()
{
    LOG_SCOPE();
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
  if( listExist )
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
      QMessageBox::critical(this, tr("Unable to insert"),
              tr("An error occured when attempting to insert data into kvalobs.\n"
                      "The message from kvalobs was\n%1").arg(QString(result->message)),
              QMessageBox::Ok, QMessageBox::NoButton);
    return;
  }
}

void HqcMainWindow::ListOK()
{
    LOG_SCOPE();
    if ( !dianaconnected ) {
#if 0
        QMessageBox::warning(this,
                             tr("Dianaforbindelse"),
                             tr("Har ikke kontakt med diana! "
                                "Du skulle kople til kommando-tjeneren via knappen underst til høyre i hqc-vinduet, "
                                "og kople diana til tjeneren via knappen i diana sin vindu."),
                             QMessageBox::Ok, QMessageBox::Ok);
#else
        mHints->addHint(tr("<h1>Dianaforbindelse</h1>"
                           "Har ikke kontakt med diana! "
                           "Du skulle kople til kommando-tjeneren via knappen underst til høyre i hqc-vinduet, "
                           "og kople diana til tjeneren via knappen i diana sin vindu."));
#endif
    }
    std::vector<int> selectedStations = lstdlg->getSelectedStations();
    if( selectedStations.empty() ) {
        QMessageBox::warning(this,
                             tr("Stasjonsvalg"),
                             tr("Ingen stasjoner er valgt!\nMinst en stasjon må velges"),
                             QMessageBox::Ok,
                             Qt::NoButton);
        return;
    }
    bool noTimes = true;
    for( int hour=0; hour<24; hour++ ) {
        if( clkdlg->hasHour(hour) )
          noTimes = false;
    }
    if ( noTimes ) {
        QMessageBox::warning(this,
                             tr("Tidspunktvalg"),
                             tr("Ingen tidspunkter er valgt!\nMinst ett tidspunkt må velges"),
                             QMessageBox::Ok,
                             Qt::NoButton);
        return;
    }

  if ( wElement.isEmpty() ) {
    QMessageBox::warning(this,
			 tr("Værelement"),
			 tr("Ingen værelement er valgt!\n"
                            "Værelement må velges"),
			  QMessageBox::Ok,
			  Qt::NoButton);
    return;
  }

  BusyIndicator busyIndicator;
  listExist = true;

  timeutil::ptime stime = timeutil::from_QDateTime(lstdlg->getStart());
  timeutil::ptime etime = timeutil::from_QDateTime(lstdlg->getEnd());

  QMap<QString, std::vector<int> >::const_iterator find = parameterGroups.find(wElement);
  if ( find ==  parameterGroups.end() ) {
      QMessageBox::critical(this, tr("Internal error"),
          tr("Configuration file does not seem to have an entry for parameter group %1").arg(wElement),
          QMessageBox::Ok, QMessageBox::NoButton);
      return;
  }

  parFind = find->size();
  std::vector<int> parameterList;

  selPar.clear();
  int kk = 0;

  for ( unsigned int jj = 0; jj < find->size(); jj++ ) {
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
  mi_foreach(int p, parameterList)
    qDebug() << "Selected: "<< p;

  isShTy = (lstdlg->getSelectedStationTypes().contains("ALL"));

  readFromData(stime, etime, selectedStations);

  // All windows are shown later, in the tileHorizontal function

  if ( lity == daLi or lity == alLi ) {

      model::KvalobsDataView * tableView = new model::KvalobsDataView(modelParam, boost::end(modelParam), this);
      tableView->setAttribute(Qt::WA_DeleteOnClose);

      dataModel =
          new model::KvalobsDataModel(
              parameterList, parMap, datalist, modeldatalist,
              ui->stID->isChecked(), ui->poID->isChecked(), ui->heID->isChecked(),
              reinserter != 0,
              tableView);

      connect(dataModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(TimeseriesOK()));
      connect(dataModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(sendAnalysisMessage()));
      connect(dataModel, SIGNAL(dataModification(const kvalobs::kvData &)), this, SLOT(saveDataToKvalobs(const kvalobs::kvData &)));

      // Functionality for hiding/showing rows in data list
      connect(ui->flID, SIGNAL(toggled(bool)), tableView, SLOT(toggleShowFlags(bool)));
      connect(ui->orID, SIGNAL(toggled(bool)), tableView, SLOT(toggleShowOriginal(bool)));
      connect(ui->moID, SIGNAL(toggled(bool)), tableView, SLOT(toggleShowModelData(bool)));

      connect(this, SIGNAL(statTimeReceived(const QString &)), tableView, SLOT(selectStation(const QString &)));

      connect(tableView, SIGNAL(stationSelected(int, const timeutil::ptime &)), this, SLOT(sendStation(int)));
      connect(tableView, SIGNAL(timeSelected(const timeutil::ptime &)), SLOT(sendObservations(const timeutil::ptime &)));
      connect(tableView, SIGNAL(parameterSelected(const QString &)), SLOT(sendSelectedParam(const QString &)));

      connect(ui->stID, SIGNAL(toggled(bool)), dataModel, SLOT(setShowStationName(bool)));
      connect(ui->poID, SIGNAL(toggled(bool)), dataModel, SLOT(setShowPosition(bool)));
      connect(ui->heID, SIGNAL(toggled(bool)), dataModel, SLOT(setShowHeight(bool)));

      tableView->setModel(dataModel);

      // Smaller cells. This should probably be done dynamically, somehow
      int columns = dataModel->columnCount();
      for ( int i = 0; i < columns; ++ i )
        tableView->setColumnWidth(i, 56);

      int rows = dataModel->rowCount();
      for ( int i = 0; i < rows; ++ i )
        tableView->setRowHeight(i,24);

      tableView->toggleShowFlags(ui->flID->isChecked());
      tableView->toggleShowOriginal(ui->orID->isChecked());
      tableView->toggleShowModelData(ui->moID->isChecked());

      ui->ws->addSubWindow(tableView);

      const QString hqc_icon_path = ::hqc::getPath(::hqc::IMAGEDIR) + "/hqc.png";
      tableView->setIcon( QPixmap(hqc_icon_path) );
      tableView->setCaption(tr("Dataliste"));
  }

  if ( lity == erLi or lity == erSa or lity == alLi ) {
      ErrorList * erl = new ErrorList(selPar,
                          stime,
                          etime,
                          this,
                          lity,
                          selParNo,
                          datalist,
                          modeldatalist,
                          userName);
      connect(ui->saveAction, SIGNAL( activated() ), erl, SLOT( saveChanges() ) );
      //      connect( erl, SIGNAL( stationSelected( int, const timeutil::ptime & ) ), tableView, SLOT(selectStation(const QString &)));
      connect( erl, SIGNAL( statSel(const miMessage&) ),
	       SLOT( processLetter(const miMessage &) ) );
      ui->ws->addSubWindow(erl);
  }

  tileHorizontal();

  vector<QString> stationList;
  int stnr=-1;
  for ( unsigned int i = 0; i < datalist->size(); i++) {
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

void HqcMainWindow::TimeseriesOK() {
  timeutil::ptime stime;
  timeutil::ptime etime;
  vector<std::string> parameter;
  vector<POptions::PlotOptions> plotoptions;
  vector<int> parameterIndex;
  vector<int> stationIndex;

  tsdlg->getResults(parameter,stime,etime,stationIndex,plotoptions);

  // make timeseries
  TimeSeriesData::tsList tslist;

  int nTypes = tsdlg->obsCheckBox->isChecked() + tsdlg->modCheckBox->isChecked();

  for ( unsigned int ip = 0; ip < parameter.size(); ip++ ) {
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
      for ( unsigned int i = 0; i < modeldatalist.size(); i++) { // fill data
	if ( modeldatalist[i].stnr == stationIndex[ip] &&
	     modeldatalist[i].otime >= stime &&
	     modeldatalist[i].otime <= etime ){
	  tseries.add(TimeSeriesData::Data(timeutil::make_miTime(modeldatalist[i].otime),
					   modeldatalist[i].orig[parameterIndex[ip]]));
	}
      }
      if(tseries.dataOK()) {
	tslist.push_back(tseries);
      }
    }
    if ( tsdlg->obsCheckBox->isChecked() && (nTypes == 1 || ip%nTypes == 0) ) {
      for ( unsigned int i = 0; i < datalist->size(); i++) { // fill data
          const timeutil::ptime& otime = (*datalist)[i].otime();
          if ( (*datalist)[i].stnr() == stationIndex[ip] &&
	     otime >= stime &&
             otime <= etime &&
	     otime.time_of_day().minutes() == 0 ) {
	  if ( (*datalist)[i].corr(parameterIndex[ip]) > -32766.0 )
	    tseries.add(TimeSeriesData::Data(timeutil::make_miTime(otime),
					     (*datalist)[i].corr(parameterIndex[ip])));
	}
      }
      if(tseries.dataOK()) {
	tslist.push_back(tseries);
      }
    }
    else if (tsdlg->modCheckBox->isChecked() && tsdlg->obsCheckBox->isChecked() ) {
      for ( unsigned int i = 0; i < modeldatalist.size(); i++) { // fill data
	if ( modeldatalist[i].stnr == stationIndex[ip] &&
	     modeldatalist[i].otime >= stime &&
	     modeldatalist[i].otime <= etime ){
	  tseries.add(TimeSeriesData::Data(timeutil::make_miTime(modeldatalist[i].otime),
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
    tsVisible = false;
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
  tsVisible = true;
}

const std::list<listStat_t>& HqcMainWindow::getStationDetails()
{
    LOG_SCOPE();
    const timeutil::ptime now = timeutil::now();
    std::cerr << "now=" << now << " last=" << mLastStationListUpdate << std::endl;
    if( mLastStationListUpdate.is_not_a_date_time()
        or (now - mLastStationListUpdate).total_seconds() > 3600 )
    {
        mLastStationListUpdate = now;
        readFromStInfoSys()
            or readFromStationFile();
    }
    return listStat;
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
  timeutil::ptime dtto = timeutil::from_YMDhms(todt.year(), todt.month(), todt.day(), toti.hour(), 0, 0);
  QDate fromdt = (txtdlg->dtfrom).date();
  QTime fromti = (txtdlg->dtfrom).time();
  timeutil::ptime dtfrom = timeutil::from_YMDhms(fromdt.year(), fromdt.month(), fromdt.day(), fromti.hour(), 0, 0);
  kvservice::WhichDataHelper whichData;
  whichData.addStation(stnr, timeutil::to_miTime(dtfrom), timeutil::to_miTime(dtto));
  GetTextData textDataReceiver(this);
  if(!kvservice::KvApp::kvApp->getKvData(textDataReceiver, whichData)){
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
    kvalobs::kvData data;

    QMdiSubWindow* subWindow = ui->ws->activeSubWindow();
    if( subWindow ) {
        ErrorList* errorList = dynamic_cast<ErrorList*>(subWindow->widget());
        if( errorList )
            data = errorList->getKvData();
    }

    WatchRR::RRDialog * rrd = WatchRR::RRDialog::getRRDialog( data, slist, this, Qt::Window );
    if ( rrd ) {
        rrd->setReinserter( reinserter );
        rrd->show();
    }
}

void HqcMainWindow::showWeather()
{
    kvalobs::kvData data;
    
    QMdiSubWindow * subWindow = ui->ws->activeSubWindow();
    if ( subWindow ) {
        ErrorList * errorList = dynamic_cast<ErrorList *>(subWindow->widget());
        if( errorList )
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
    int noDays = lstdlg->getStart().daysTo(lstdlg->getEnd());
    if ( noDays < 27 ) {
      mx = mx.addSecs(-60*mx.time().minute());
      mx = mx.addSecs(3600);
      lstdlg->setEnd(mx);
    }

    lstdlg->showAll();
  }
}

void HqcMainWindow::clockMenu()
{
    clkdlg->hideAll();
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
  // UNUSED int parameterIndex;
  bool maybeQC2;
  bool result = actsdlg->getResults(parameter,stime,etime,stationIndex, maybeQC2);
  if ( !result ) return;
  std::list<kvalobs::kvParam>::const_iterator it=plist.begin();
  for(;it!=plist.end(); it++){
    if ( it->name().c_str() == parameter ){
      // UNUSED parameterIndex = it->paramID();
      break;
    }
  }
  kvservice::WhichDataHelper whichData;
  long int stnr = stationIndex;
  boost::posix_time::ptime ft = timeutil::from_iso_extended_string(stime.toString("yyyy-MM-dd hh:mm:ss").toStdString());
  boost::posix_time::ptime tt = timeutil::from_iso_extended_string(etime.toString("yyyy-MM-dd hh:mm:ss").toStdString());
  whichData.addStation(stnr, timeutil::to_miTime(ft), timeutil::to_miTime(tt));
  checkTypeId(stnr);
  int firstRow = dataModel->dataRow(stnr, ft, model::KvalobsDataModel::OBSTIME_AFTER );
  int lastRow  = dataModel->dataRow(stnr, tt, model::KvalobsDataModel::OBSTIME_BEFORE);
  int column   = dataModel->dataColumn(parameter);

  QString ch;
  vector<QString> chList;
  vector<double> newCorr;
  for ( int irow = firstRow; irow <= lastRow; irow++) {
    QModelIndex index = dataModel->index(irow, column);
    const kvalobs::kvData & dt = dataModel->getKvData_(index);
    //    if ( dt.corrected() < -32760 )
    //      continue;
    QString ori;
    ori = ori.setNum(dt.original(), 'f', 1);
    QString stnr;
    stnr = stnr.setNum(dt.stationID());
    ch = stnr + " " + QString::fromStdString(timeutil::to_iso_extended_string(timeutil::from_miTime(dt.obstime())))
        + ": " + parMap[dt.paramID()] + ": " + ori;
    chList.push_back(ch);
    newCorr.push_back(dt.original());
  }
  ApproveDialog* approveDialog = new ApproveDialog(chList);
  approveDialog->setWindowTitle(tr("%1 - Godkjenning av data").arg(QApplication::applicationName()));
  int res = approveDialog->exec();
  if ( res == QDialog::Accepted )
  for ( int irow = firstRow; irow <= lastRow; irow++) {
    QModelIndex index = dataModel->index(irow, column);
    dataModel->setAcceptedData(index, newCorr[irow-firstRow], maybeQC2);
    const kvalobs::kvData & dt = dataModel->getKvData_(index);
    std::list<kvalobs::kvData> modData;
    modData.push_back( dt );
    CKvalObs::CDataSource::Result_var result;
    {
      //      BusyIndicator busyIndicator;
      result = reinserter->insert( modData );
    }
    modData.clear();
  }
  return;
}

void HqcMainWindow::rejectTimeseriesOK() {
  QDateTime stime;
  QDateTime etime;
  QString parameter;
  int stationIndex;
  // UNUSED int parameterIndex;
  bool result = rjtsdlg->getResults(parameter,stime,etime,stationIndex);
  if ( !result ) return;
  std::list<kvalobs::kvParam>::const_iterator it=plist.begin();
  for(;it!=plist.end(); it++){
    if ( it->name().c_str() == parameter ){
        // UNUSED parameterIndex = it->paramID();
      break;
    }
  }
  kvservice::WhichDataHelper whichData;
  long int stnr = stationIndex;
  boost::posix_time::ptime ft = timeutil::from_iso_extended_string(stime.toString("yyyy-MM-dd hh:mm:ss").toStdString());
  boost::posix_time::ptime tt = timeutil::from_iso_extended_string(etime.toString("yyyy-MM-dd hh:mm:ss").toStdString());
  whichData.addStation(stnr, timeutil::to_miTime(ft), timeutil::to_miTime(tt));
  checkTypeId(stnr);
  int firstRow = dataModel->dataRow(stnr, ft, model::KvalobsDataModel::OBSTIME_AFTER );
  int lastRow  = dataModel->dataRow(stnr, tt, model::KvalobsDataModel::OBSTIME_BEFORE);
  int column   = dataModel->dataColumn(parameter);

  QString ch;
  vector<QString> chList;
  for ( int irow = firstRow; irow <= lastRow; irow++) {
    QModelIndex index = dataModel->index(irow, column);
    const kvalobs::kvData & dt = dataModel->getKvData_(index);
    if ( dt.corrected() < -32760 )
      continue;
    QString cr;
    cr = cr.setNum(dt.corrected(), 'f', 1);
    QString stnr;
    stnr = stnr.setNum(dt.stationID());
    ch = stnr + " " + QString::fromStdString(timeutil::to_iso_extended_string(timeutil::from_miTime(dt.obstime())))
        + ": " + parMap[dt.paramID()] + ": " + cr;
    chList.push_back(ch);
  }
  DiscardDialog* discardDialog = new DiscardDialog(chList);
  discardDialog->setWindowTitle(tr("%1 - Forkasting av data").arg(QApplication::applicationName()));
  int res = discardDialog->exec();
  if ( res == QDialog::Accepted )
  for ( int irow = firstRow; irow <= lastRow; irow++) {
    QModelIndex index = dataModel->index(irow, column);
    dataModel->setDiscardedData(index, -32766);
    const kvalobs::kvData & dt = dataModel->getKvData_(index);
    std::list<kvalobs::kvData> modData;
    modData.push_back( dt );
    CKvalObs::CDataSource::Result_var result;
    {
      //      BusyIndicator busyIndicator;
      result = reinserter->insert( modData );
    }
    modData.clear();
  }
  return;
}

void HqcMainWindow::startKro() {
    QDesktopServices::openUrl(QUrl("http://kro/cgi-bin/start.pl"));
}

void HqcMainWindow::screenshot()
{
    QPixmap hqcPixmap = QPixmap();
    hqcPixmap = QPixmap::grabWidget(this);
    
    QPrinter printer;
    printer.setPaperSize(QPrinter::A4);
    printer.setOrientation(QPrinter::Landscape);

    QPrintDialog *dialog = new QPrintDialog(&printer, this);
    dialog->setWindowTitle(tr("Utskrift skjermbilde"));
    if (dialog->exec() != QDialog::Accepted)
        return;

    QPainter painter(&printer);
    QRect rect = painter.viewport();
    QSize size = hqcPixmap.size();
    size.scale(rect.size(), Qt::KeepAspectRatio);
    painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
    painter.setWindow(hqcPixmap.rect());
    painter.drawImage(0,0,hqcPixmap);
}

void HqcMainWindow::onVersionCheckTimeout()
{
    QFile versionFile(::hqc::getPath(::hqc::CONFDIR) + "/../hqc_current_version");
    if (versionFile.open(QIODevice::ReadOnly)) {
        QTextStream in(&versionFile);
        if( !in.atEnd() ) {
            const long installedVersion = in.readLine().toLong();
            const long runningVersion = PVERSION_NUMBER_MAJOR_MINOR_PATCH;
            if( installedVersion > runningVersion ) {
                QMessageBox::information(this,
                                         tr("HQC - Oppdatering"),
                                         tr("Hqc-applikasjonen ble oppdatert på din datamaskin. "
                                            "Du skulle lagre eventuelle endringer og starte "
                                            "hqc-applikasjonen på nytt for å bruke den nye versjonen."),
                                         QMessageBox::Ok, Qt::NoButton);
            } else {
                //std::cout << "no update, now=" << runningVersion << " installed=" << installedVersion << std::endl;
            }
            mVersionCheckTimer->start(VERSION_CHECK_TIMEOUT);
            return;
        }
    }
    // something went wrong when reading the version info file
    std::cout << "error reading share/.../hqc_current_version, not renewing timer" << std::endl;
}

void HqcMainWindow::closeEvent(QCloseEvent* event)
{
  writeSettings();
  QWidget::closeEvent(event);
}

HqcMainWindow::~HqcMainWindow()
{
}

bool HqcMainWindow::timeFilter(int hour)
{
    return clkdlg->hasHour(hour);
}

bool HqcMainWindow::hqcTypeFilter(int typeId, int environment, int /* UNUSED stnr*/)
{
    const QStringList stationTypes = lstdlg->getSelectedStationTypes();
  if ( typeId == -1 || typeId == 501 ) return false;
  //  if ( typeId == -1 ) return false;
  if ( lstdlg->showSynop() or lstdlg->showPrioritized() ) return true;
  int atypeId = typeId < 0 ? -typeId : typeId;
  // FIXME this needs to match ListDialog.cc: StationTable::StationTable
  if ( stationTypes.contains("ALL") ) return true;
  if ( environment == 1 && atypeId == 311 && stationTypes.contains("AF") ) return true;
  if ( environment == 8 && (atypeId == 3 || atypeId == 311 || atypeId == 412 || atypeId == 330 || atypeId == 342) && stationTypes.contains("AA") ) return true;
  if ( environment == 2 && atypeId == 3 && stationTypes.contains("AL") ) return true;
  if ( environment == 12 && atypeId == 3 && stationTypes.contains("AV") ) return true;
  if ( atypeId == 410 && stationTypes.contains("AO") ) return true;
  if ( environment == 7 && stationTypes.contains("MV") ) return true;
  if ( environment == 5 && stationTypes.contains("MP") ) return true;
  if ( environment == 4 && stationTypes.contains("MM") ) return true;
  if ( environment == 6 && stationTypes.contains("MS") ) return true;
  if ( (atypeId == 4 || atypeId == 404) && stationTypes.contains("P") ) return true;
  if ( (atypeId == 4 || atypeId == 404)&& stationTypes.contains("PT") ) return true;
  if ( atypeId == 302 && stationTypes.contains("NS") ) return true;
  if ( environment == 9 && atypeId == 402 && stationTypes.contains("ND") ) return true;
  if ( environment == 10 && atypeId == 402 && stationTypes.contains("NO") ) return true;
  if ( (atypeId == 1 || atypeId == 6 || atypeId == 312 || atypeId == 412) && stationTypes.contains("VS") ) return true;
  if ( environment == 3 && atypeId == 412 && stationTypes.contains("VK") ) return true;
  if ( (atypeId == 306 || atypeId == 308 || atypeId == 412) && stationTypes.contains("VM") ) return true;
  if ( atypeId == 2 && stationTypes.contains("FM") ) return true;
  return false;
}

bool HqcMainWindow::typeIdFilter(int stnr, int typeId, int sensor, const timeutil::ptime& otime, int par) {
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
    int hr = otime.time_of_day().hours();
    int dg = otime.date().day();
    int mn = otime.date().month();
    int ar = otime.date().year();
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
      int hr = otime.time_of_day().hours();
      int dg = otime.date().day();
      int mn = otime.date().month();
      int ar = otime.date().year();
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
  mi_foreach(const currentType& ct, currentTypeList) {
    if ( stnr == ct.stnr &&
	 abs(typeId) == ct.cTypeId &&
	 sensor == ct.cSensor &&
	 par == ct.par &&
	 (ct.fDate.is_not_a_date() || otime.date() >= ct.fDate) &&
	 (ct.tDate.is_not_a_date() || otime.date() <= ct.tDate) ) {
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
void HqcMainWindow::readFromData(const timeutil::ptime& stime,
				 const timeutil::ptime& etime,
				 const std::vector<int>& stList) {
  BusyIndicator busy();

  kvservice::WhichDataHelper whichData;
  for ( unsigned int i = 0; i < stList.size(); i++ ) {
    whichData.addStation(stList[i], timeutil::to_miTime(stime), timeutil::to_miTime(etime));
    checkTypeId(stList[i]);
  }

  // Throw away old data list. It will still exist if any window need it.
  datalist = model::KvalobsDataListPtr(new model::KvalobsDataList);
  GetData dataReceiver(this);

  if( kvservice::KvApp::kvApp->getKvData(dataReceiver, whichData) ) {
      // this will make calls to HqcMainWindow::makeObsDataList
  } else {
    std::cerr << "problems retrieving data" << std::endl;
  }


  ModelDataList mdlist;
  modeldatalist.reserve(131072);
  modeldatalist.clear();

  if(!kvservice::KvApp::kvApp->getKvModelData(mdlist, whichData))
      cerr << "Can't connect to modeldata table!" << endl;
  modDatl mtdl;
  CIModelDataList it=mdlist.begin();
  if (it != mdlist.end()) {
      timeutil::ptime protime = timeutil::from_miTime(it->obstime());
      int prstnr = it->stationID();
      mtdl.otime = protime;
      mtdl.stnr  = prstnr;
      mtdl.orig[it->paramID()] = it->original();
      for (++it; it != mdlist.end(); ++it) {
          int stnr = it->stationID();
          timeutil::ptime otime = timeutil::from_miTime(it->obstime());

          if (otime != protime || (otime == protime && stnr != prstnr)) {
              modeldatalist.push_back(mtdl);
              mtdl = modDatl();
          }

          mtdl.otime = otime;
          mtdl.stnr  = stnr;
          mtdl.orig[it->paramID()] = it->original();
          protime = otime;
          prstnr = stnr;
      }
  }
}

/*!
 Read station info from the stinfosys database
*/
bool HqcMainWindow::readFromStInfoSys()
{
    const int webs_ids[] = {
        4780, 7010, 10380, 16610, 17150, 18700, 23420, 24890, 27500,
        31620, 36200, 39100, 42160, 44560, 47300, 50500, 50540, 54120,
        59800, 60500, 69100, 71550, 75410, 75550, 80610, 82290, 85380,
        87110, 89350, 90450, 93140, 93700, 94500, 96400, 98550, 99370,
        99710, 99720, 99840, 99910, 99950 };
    const std::set<int> webs(webs_ids, boost::end(webs_ids));

    const int pri1s_ids[] = {
        4780, 10380, 18500, 18700, 24890, 27500, 36200, 39040, 44560,
        50540, 54120, 60500, 69100, 70850, 72800, 82290, 90450, 93700,
        97251, 98550, 99710, 99720, 99840, 99910, 99950 };
    const std::set<int> pri1s(pri1s_ids, boost::end(pri1s_ids));

    const int pri2s_ids[] = {
        3190, 17150, 28380, 35860, 60990, 63420, 68860, 68863, 93140, 94500, 95350 };
    const std::set<int> pri2s(pri2s_ids, boost::end(pri2s_ids));
    
    const int pri3s_ids[] = {
        180, 700, 1130, 2540, 4440, 4460, 6020, 7010, 8140, 9580, 11500, 12320,
         12550, 12680, 13160, 13420, 13670, 15730, 16610, 16740, 17000, 18950,
         19710, 20301, 21680, 23160, 23420, 23500, 23550, 25110, 25830, 26900,
         26990, 27450, 27470, 28800, 29720, 30420, 30650, 31620, 32060, 33890,
         34130, 36560, 37230, 38140, 39100, 39690, 40880, 41110, 41670, 41770,
         42160, 42920, 43010, 44080, 44610, 45870, 46510, 46610, 46910, 47260,
         47300, 48120, 48330, 49800, 50070, 50300, 50500, 51530, 51800, 52290,
         52535, 52860, 53101, 55290, 55700, 55820, 57420, 57710, 57770, 58070,
         58900, 59110, 59610, 59680, 59800, 61180, 61770, 62270, 62480, 63705,
         64330, 64550, 65310, 65940, 66730, 68340, 69150, 69380, 70150, 71000,
         71850, 71990, 72060, 72580, 73500, 75220, 75410, 75550, 76330, 76450,
         76530, 76750, 77230, 77550, 78800, 79600, 80101, 80102, 80610, 80700,
         81680, 82410, 83550, 85380, 85450, 85891, 85892, 86500, 86740, 87110,
         87640, 88200, 88690, 89350, 90490, 90800, 91380, 91760, 92350, 93301,
         93900, 94280, 94680, 96310, 96400, 96800, 97350, 98090, 98400, 98790,
         99370, 99760, 99765 };
    const std::set<int> pri3s(pri3s_ids, boost::end(pri3s_ids));
    
    const int coast_ids[] = {
        17000, 17280, 17290, 27080, 27150, 27230, 27240, 27270, 27410, 27490,
        27500, 29950, 30000, 34080, 34120, 34130, 35850, 35860, 36150, 36200,
        38110, 39040, 39100, 39170, 39350, 41090, 41100, 41110, 41750, 41760,
        41770, 41772, 42160, 43270, 43340, 43350, 43900, 44080, 44320, 44560,
        44600, 44610, 44630, 44640, 45870, 45880, 45900, 47190, 47200, 47210,
        47260, 47300, 48120, 48300, 48330, 50460, 50500, 50540, 50541, 50542,
        50543, 50544, 50560, 50700, 52530, 52535, 52800, 52860, 56420, 56440,
        57710, 57720, 57740, 57750, 57760, 57770, 59100, 59110, 59580, 59610,
        59800, 59810, 60830, 60945, 60950, 60990, 61040, 61060, 61150, 61170,
        61180, 62270, 62300, 62310, 62480, 62490, 62500, 62640, 62650, 62660,
        64250, 64260, 64330, 65300, 65310, 65340, 65370, 65450, 65720, 65940,
        65950, 71540, 71550, 71560, 71650, 71780, 71850, 71980, 71990, 75220,
        75300, 75350, 75410, 75550, 75600, 75700, 76300, 76310, 76320, 76330,
        76450, 76500, 76530, 76600, 76750, 76810, 76820, 76850, 76900, 76920,
        76923, 76925, 76926, 76928, 76930, 76931, 76932, 76933, 76940, 76941,
        76942, 76943, 76944, 76945, 76946, 76947, 76948, 76949, 76950, 76951,
        76970, 76971, 76974, 76978, 76980, 76985, 77002, 77005, 77006, 77020,
        77021, 77022, 77023, 77024, 80100, 80101, 80102, 80300, 80610, 80740,
        80900, 80950, 82240, 82250, 82260, 82270, 82280, 82290, 82400, 82410,
        83120, 83550, 83700, 83710, 85030, 85040, 85230, 85240, 85380, 85450,
        85460, 85560, 85700, 85780, 85840, 85890, 85891, 85900, 85910, 85950,
        86150, 86200, 86230, 86260, 86500, 86520, 86600, 86740, 86750, 86760,
        86780, 87100, 87110, 87111, 87120, 87350, 87400, 87410, 87420, 87640,
        88580, 88680, 88690, 90280, 90400, 90440, 90450, 90490, 90500, 90700,
        90720, 90721, 90760, 90800, 90900, 91190, 92700, 92750, 93000, 94250,
        94260, 94280, 94350, 94450, 94500, 94600, 94680, 94700, 95800, 96300,
        96310, 96400, 96550, 98090, 98360, 98400, 98550, 98580, 98700, 98790,
        98800, 99710, 99720, 99735, 99737, 99754, 99765, 99790, 99821, 99950,
        99970 };
    const std::set<int> coast(coast_ids, boost::end(coast_ids));

    const int N_FOREIGN = 13;
    const int foreignId[N_FOREIGN] = {
        99986, 99990, 202000, 203600, 208000, 210400, 211000,
        220600, 222200, 230800, 241800, 250000, 280700 };
    const QString foreignName[N_FOREIGN] = {
        " ", " ", "NORDLAND", "NORDLAND", "FINNMARK", "NORDLAND", "NORDLAND",
        "NORD-TRØNDELAG", "NORD-TRØNDELAG", "SØR-TRØNDELAG", "ØSTFOLD",
        "ØSTFOLD", "FINNMARK" };
    
    if (!connect2stinfosys()) {
        cerr << "Cannot connect to stinfosys" << endl;
        return false;
    }
    const char stinfosys_SQL[] =
        "SELECT DISTINCT s.stationid, m.name,"
        "       (SELECT mm.name FROM municip mm"
        "        WHERE mm.municipid BETWEEN 1 AND 100"
        "              AND ((m.municipid NOT IN (2300,2800) AND mm.municipid = m.municipid / 100)"
        "                   OR (m.municipid IN (2300,2800) AND mm.municipid = m.municipid)))"
        " FROM station s, municip m"
        " WHERE s.stationid BETWEEN 60 AND 99999"
        "   AND s.municipid = m.municipid"
        " ORDER by s.stationid";
    QSqlQuery query;
    if( not query.exec(stinfosys_SQL) ) {
        cerr << "query to stinfosys failed: " << query.lastError().text().toStdString() << endl;
        return false;
    }
    typedef std::map<int,countyInfo> cList_t;
    cList_t cList;
    std::cerr << "got " << query.size() << " station city/county names from stinfosys" << std::endl;
    while( query.next() ) {
        countyInfo cInfo;
        cInfo.stnr    = query.value(0).toInt();
	cInfo.municip = query.value(1).toString();
        cInfo.county  = query.value(2).toString();

        if( cInfo.county == "SVALBARD" || cInfo.county == "JAN MAYEN" )
            cInfo.county = "ISHAVET";

        if( webs.find(cInfo.stnr) != webs.end() )
            cInfo.web = "WEB";
        if( pri3s.find(cInfo.stnr) != pri3s.end() )
            cInfo.pri = "PRI3";
        else if( pri2s.find(cInfo.stnr) != pri2s.end() )
            cInfo.pri = "PRI2";
        else if( pri1s.find(cInfo.stnr) != pri1s.end() )
            cInfo.pri = "PRI1";

        if( coast.find(cInfo.stnr) != coast.end() )
            cInfo.ki = "K";
        else
            cInfo.ki = "I";
          
        cList[cInfo.stnr] = cInfo;
    }
    
    // some additional stations
    for( int i=0; i<N_FOREIGN; i++ ) {
        countyInfo cInfo;
        cInfo.stnr   = foreignId[i];
        cInfo.county = foreignName[i];
        cInfo.ki = "I";
        cList[cInfo.stnr] = cInfo;
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
    listStat.clear();

    mi_foreach(const kvalobs::kvStation& st, slist) {
      cList_t::const_iterator cit = cList.find(st.stationID());
      if( cit == cList.end() ) {
          if( st.stationID() < 100000 )
              std::cerr << "station " << st.stationID() << " from kvalobs' station table not found in stinfosys" << std::endl;
          continue;
      }
      const countyInfo& ci = cit->second;
      QString strStnr;
      QString strHoh;
      QString strEnv;
      strEnv = strEnv.setNum(st.environmentid());

      listStat_t ls;
      ls.name        = st.name();
      ls.stationid   = st.stationID();
      ls.altitude    = st.height();
      ls.environment = st.environmentid();
      ls.fylke       = ci.county.toStdString();
      ls.kommune     = ci.municip.toStdString();
      ls.wmonr       = ci.web.toStdString();
      ls.pri         = ci.pri.toStdString();
      ls.coast       = ci.ki == "K";
      listStat.push_back(ls);

      std::ostringstream obuf;
      obuf << setw(7) << right << ci.stnr << " "
	   << setw(31) << left << (ci.county).toStdString()
	   << setw(25) << (ci.municip).toStdString()
	   << setw(3) << (ci.web).toStdString()
	   << setw(4) << (ci.pri).toStdString()
	   << setw(1) << (ci.ki).toStdString();
      outf << obuf.str() << std::endl;
      //cout << obuf.str() << std::endl;

    }
    outf.close();
    cout << "Stationliste hentet fra stinfosys" << endl;
    return true;
}

void HqcMainWindow::exitNoKvalobs()
{
    QMessageBox msg(this);
    msg.setIcon(QMessageBox::Critical);
    msg.setText(tr("Kvalobsdatabasen er dessverre ikke tilgjengelig."));
    msg.setInformativeText(tr("HQC avsluttes fordi den kan ikke brukes uten kvalobs-databasen."));
    msg.exec();
    exit(1);
}

/*!
 Read the station table in the kvalobs database
*/
void HqcMainWindow::readFromStation()
{
    if (!kvservice::KvApp::kvApp->getKvStations(slist))
        exitNoKvalobs();
}

/*!
 Read the obs_pgm table in the kvalobs database
*/
void HqcMainWindow::readFromObsPgm() {
    LOG_SCOPE();

    obsPgmList.clear();
    const int N_STATIONS = 100;
    std::list<kvalobs::kvStation>::const_iterator it = slist.begin();
    while (it != slist.end()) {
        std::list<long> statList;
        for(int i=0; i<N_STATIONS and it != slist.end(); ++i, ++it)
            statList.push_back(it->stationID());
        if (not statList.empty()) {
            std::list<kvalobs::kvObsPgm> obsPgmTmp;
            if (not kvservice::KvApp::kvApp->getKvObsPgm(obsPgmTmp, statList, false))
                exitNoKvalobs();
            obsPgmList.insert(obsPgmList.end(), obsPgmTmp.begin(), obsPgmTmp.end());
        }
    }

    int prostnr = -1;

    TypeList tpList;
    mi_foreach(const kvalobs::kvObsPgm& op, obsPgmList) {
        const int ostnr = op.stationID();
        if ( ostnr != prostnr ) {
            if ( prostnr != -1 )
                otpList.push_back(tpList);
            tpList.clear();
            tpList.push_back(ostnr);
        }
        int otpid = op.typeID();
        TypeList::iterator tpind = std::find(tpList.begin(), tpList.end(), otpid);
        if ( tpind ==  tpList.end() )
            tpList.push_back(otpid);
        prostnr = ostnr;
    }
    otpList.push_back(tpList);
}

/*!
 Read the typeid file
*/
void HqcMainWindow::checkTypeId(int stnr) {
  mi_foreach(const kvalobs::kvObsPgm& op, obsPgmList) {
    int stationId = op.stationID();
    if ( stationId == stnr ) {
      currentType crT;
      crT.stnr    = stationId;
      crT.par     = op.paramID();
      crT.fDate   = timeutil::from_miTime(op.fromtime()).date();
      crT.tDate   = timeutil::from_miTime(op.totime()).date();;
      crT.cLevel  = op.level();
      crT.cSensor = 0;
      crT.cTypeId = op.typeID();
      currentTypeList.push_back(crT);
    }
  }
}

/*!
 Read the station file, this must be done after the station table in the database is read
*/
bool HqcMainWindow::readFromStationFile()
{
  QString path = QString(getenv("HOME"));
  if ( path.isEmpty() ) {
    cerr << "Intet environment" << endl;
    exit(1);
  }
  QString stationFile = path + "/.config/hqc_stations";
  QFile stations(stationFile);
  stations.open(QIODevice::ReadOnly);
  QTextStream stationStream(&stations);
  // UNUSED int i = 0;
  int prevStnr = 0;
  while ( stationStream.atEnd() == 0 ) {
    QString statLine = stationStream.readLine();
    QString qwert = statLine.mid(64,3);
    int stnr = statLine.left(7).toInt();
    if ( stnr == prevStnr )
        continue;

    std::list<kvalobs::kvStation>::const_iterator it = std::find_if(slist.begin(), slist.end(), kvStationById(stnr));
    if(it != slist.end()) {
      QString strStnr;
      QString strHoh;
      QString strEnv;
      strEnv = strEnv.setNum(it->environmentid());
      QString strSnr("    ");
      if(it->wmonr() > 0)
        strSnr = strSnr.setNum(it->wmonr());

      listStat_t ls;
      ls.name        = it->name();
      ls.stationid   = it->stationID();
      ls.altitude    = it->height();
      ls.environment = it->environmentid();
      ls.fylke       = statLine.mid(8,30).stripWhiteSpace().toStdString();
      ls.kommune     = statLine.mid(39,24).stripWhiteSpace().toStdString();
      ls.wmonr       = it->wmonr();
      ls.pri         = statLine.mid(67,4).stripWhiteSpace().toStdString();
      ls.coast       = statLine.mid(71,1).stripWhiteSpace() == "K";
      listStat.push_back(ls);
    }
    prevStnr = stnr;
  }
  return true;
}

/*!
 Read the param table in the kvalobs database
*/
void HqcMainWindow::readFromParam()
{
    LOG_SCOPE();

    // First, read parameter order from file
    QString fileParamOrder = ::hqc::getPath(::hqc::CONFDIR) + "/paramorder";
    QFile paramOrder(fileParamOrder);
    if( !paramOrder.open(QIODevice::ReadOnly) ) {
        QMessageBox::critical(this,
                              tr("Cannot read paramorder file"),
                              tr("The file expected in '%1' could not be opened. Please set HQC_CONFDIR correctly.").arg(fileParamOrder),
                              QMessageBox::Abort,
                              Qt::NoButton);
        ::exit(1);
    }

    QTextStream paramStream(&paramOrder);
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

    if(!kvservice::KvApp::kvApp->getKvParams(plist))
        cerr << "Can't connect to param table!" << endl;
    
    mi_foreach(const kvalobs::kvParam& p, plist)
        parMap[p.paramID()] = QString::fromStdString(p.name());
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
    std::list<kvalobs::kvStation>::const_iterator it = std::find_if(slist.begin(), slist.end(), kvStationById(stnr));
    if( it != slist.end() ) {
      name = QString(it->name().c_str());
      lat  = (it->lat());
      lon  = (it->lon());
      hoh  = (it->height());
      snr  = (it->wmonr());
      env  = (it->environmentid());
    }
}

void HqcMainWindow::findStationPos(int stnr, double& lat, double& lon, double& hoh)
{
    QString dummyName;
    int dummyWMOnr, dummyEnvironmentId;
    findStationInfo(stnr, dummyName, lat, lon, hoh, dummyWMOnr, dummyEnvironmentId);
}

void HqcMainWindow::tileHorizontal() {

//  ws->tileSubWindows();

  // primitive horizontal tiling
  QList<QMdiSubWindow *> windows = ui->ws->subWindowList();
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
    int height[] = {0, 28 + ui->ws->height() / 2, (ui->ws->height() / 2) - 28} ;

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

      window->setGeometry( 0, y, ui->ws->width(), actHeight );
      y += actHeight;
    }
  }
}

void HqcMainWindow::closeWindow()
{
  firstObs=true;
  cerr << "HqcMainWindow::closeWindow()\n";

  ui->ws->closeActiveSubWindow();

  emit windowClose();
}


void HqcMainWindow::helpUse() {
    QDesktopServices::openUrl(QUrl("https://dokit.met.no/klima/tools/qc/hqc-help"));
}

void HqcMainWindow::helpFlag() {
    QDesktopServices::openUrl(QUrl("https://kvalobs.wiki.met.no/doku.php?id=kvalobs:kvalobs-flagg"));
}

void HqcMainWindow::helpParam() {
    QDesktopServices::openUrl(QUrl("https://kvalobs.wiki.met.no/doku.php?id=kvalobs:kvalobs-parametre_sortert_alfabetisk_etter_kode"));
}

void HqcMainWindow::about()
{
    QMessageBox::about( this, tr("Om Hqc"),
			tr("Hqc er et program for manuell kvalitetskontroll av observasjoner."
                           "Programmet består av editerbare tabeller med observasjoner samt"
                           "tidsseriediagram, og har forbindelse med Diana."
                           "\n\n"
                           "Programmet utvikles av "
                           "Knut Johansen, "
                           "Alexander Bürger, "
                           "Lisbeth Bergholt, "
                           "Vegard Bønes, "
                           "Audun Christoffersen i met.no.\n\n"
                           "Du bruker HQC versjon %1.").arg(PVERSION_FULL));
}

void HqcMainWindow::initDiana()
{
    LOG_SCOPE();
    dianaconnected= false;

    if(pluginB->clientTypeExist("Diana"))
        dianaconnected= true;

    sendTimes();
    sendAnalysisMessage();
}

// called when client-list changes
void HqcMainWindow::processConnect()
{
  LOG_SCOPE();
  initDiana();
}

void HqcMainWindow::cleanConnection()
{
  dianaconnected = false;
  firstObs = true;
  cout << "< DISCONNECTING >" << endl;
}

void HqcMainWindow::sendTimes(){
  LOG_SCOPE();

  //send times
  miMessage m;
  m.command= qmstrings::settime;
  m.commondesc = "datatype";
  m.common = "obs";
  m.description= "time";
  std::set<timeutil::ptime> allTimes;
  mi_foreach(const model::KvalobsData& d, *datalist) {
      allTimes.insert(d.otime());
  }
  mi_foreach(const timeutil::ptime& t, allTimes) {
      m.data.push_back(timeutil::to_iso_extended_string(t));
  }
  cerr << "HQC sender melding : " << m.content() << endl;
  pluginB->sendMessage(m);
}

#ifdef METLIBS_BEFORE_4_9_5
void HqcMainWindow::processLetterOld(miMessage& letter)
{
    processLetter(letter);
}
#endif

// processes incoming miMessages
void HqcMainWindow::processLetter(const miMessage& letter)
{
  LOG_SCOPE();
  qDebug() << "command=" << letter.command.c_str();
  if(letter.command == qmstrings::newclient) {
      vector<std::string> desc, valu;
      const std::string cd(letter.commondesc), co(letter.common);
      boost::split(desc, cd, boost::is_any_of(":"));
      boost::split(valu, co, boost::is_any_of(":"));
      for(vector<std::string>::const_iterator itD=desc.begin(), itC=valu.begin(); itD != desc.end(); ++itC, ++itD) {
          if( *itD == "type" && *itC == "Diana" ) {
              firstObs = true;
              processConnect();
              return;
          }
      }
  }
  else if (letter.command == "station" ) {
    const char* ccmn = letter.common.c_str();
    QString cmn = QString(ccmn);
    emit statTimeReceived(cmn);
  }
  else if(letter.command == qmstrings::timechanged){
      const char* ccmn = letter.common.c_str();
      QString cmn = QString(ccmn);
      /*emit*/ statTimeReceived(cmn);

    timeutil::ptime newTime = timeutil::from_iso_extended_string(letter.common);
    sendObservations(newTime,false);
  }
}

// send message to show ground analysis in Diana
void HqcMainWindow::sendAnalysisMessage() {
  LOG_SCOPE();

  //show analysis
  miMessage letter;
  letter.command=qmstrings::apply_quickmenu;
  letter.data.push_back("Hqc");
  letter.data.push_back("Bakkeanalyse");
    cerr << "HQC sender melding : " << letter.content() << endl;
  pluginB->sendMessage(letter);

  dianaTime = timeutil::from_iso_extended_string("2000-01-01 00:00:00");
}

void HqcMainWindow::sendStation(int stnr)
{
  LOG_SCOPE();

  miMessage pLetter;
  pLetter.command = qmstrings::station;
  const std::string stationstr = boost::lexical_cast<std::string>(stnr);
  pLetter.common = stationstr;
    cerr << "HQC sender melding : " << pLetter.content() << endl;
  pluginB->sendMessage(pLetter);

}

void HqcMainWindow::aboutQt()
{
    QMessageBox::aboutQt(this);
}

void HqcMainWindow::sendObservations(const timeutil::ptime& time, bool sendtime)
{
  LOG_SCOPE();

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
    tLetter.common = timeutil::to_iso_extended_string(time);
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
    const std::string diPar = dianaName(selPar[i].latin1());
    if( not diPar.empty() ) {
      parName.append(diPar.c_str());
      parSynop[i] = true;
      parModel[i] = mdMap[diPar];
      parDiff[i]  = diMap[diPar];
      parProp[i]  = prMap[diPar];
    }
  }

  std::string synopDescription = "id,St.type,auto,lon,lat,";
  if ( not parName.empty() )
    synopDescription += parName.join(",").latin1();
  const std::string enkelDescription = synopDescription;

  std::vector<std::string> synopData;
  std::vector<std::string> enkelData;

  int prStnr = 0;

  for ( unsigned int i = 0; i < datalist->size(); i++) { // fill data
    if ( (*datalist)[i].otime() == time || (firstObs && (*datalist)[i].stnr() != prStnr )){
      double lat,lon,h;
      std::string str = boost::lexical_cast<std::string>((*datalist)[i].stnr());
      QString name;
      int env;
      int snr;
      //      int typeId = (*datalist)[i].typeId;
      int typeId = (*datalist)[i].showTypeId();
      findStationInfo((*datalist)[i].stnr(),name,lat,lon,h,snr,env);
      str += ",";
      std::string isAuto = "x";
      std::string strType = hqcType(typeId, env);
      if ( strType.c_str()[0] == 'A' )
	isAuto = "a";
      else if ( strType.c_str()[0] == 'N' || strType.c_str()[0] == 'P' )
	isAuto = "n";
      if ( (isAuto == "a" && strType.c_str()[1] == 'A') ||
	   (strType.c_str()[0] == 'V' && strType.c_str()[1] != 'M') ||
	   (strType == "none")  )
    	str += "none";
      else if ( strType.c_str()[0] == 'P' )
	str += strType.c_str()[0];
      else
	str += strType.c_str()[1];
      str += ",";
      str += isAuto;
      str += ",";

      str += (boost::format("%1$.4f,%2$.4f") % lat % lon).str();
      std::string synopStr = str, enkelStr = str;
      double aa = (*datalist)[i].corr(1);
      for(unsigned int j=0; j<parIndex.size();j++){
	double corr = (*datalist)[i].corr(parIndex[j]);
	if ( parModel[j] ) {
	  for ( unsigned int k = 0; k < modeldatalist.size(); k++ ) {
	    if ( modeldatalist[k].stnr == (*datalist)[i].stnr() &&
		 modeldatalist[k].otime == (*datalist)[i].otime() ) {
	      corr = modeldatalist[k].orig[parIndex[j]];
	      break;
	    }
	  }
	}
	if ( parDiff[j] ) {
	  for ( unsigned int k = 0; k < modeldatalist.size(); k++ ) {
	    if ( modeldatalist[k].stnr == (*datalist)[i].stnr() &&
		 modeldatalist[k].otime == (*datalist)[i].otime() ) {
	      corr = corr - modeldatalist[k].orig[parIndex[j]];
	      break;
	    }
	  }
	}
	if ( parProp[j] ) {
	  for ( unsigned int k = 0; k < modeldatalist.size(); k++ ) {
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
       	  std::string flagstr = (boost::format("%1$05d") % flag).str();
       	  std::string colorstr;
	  if ( maxFlag == 0 )
       	    colorstr = ";0:0:0";
	  //	  else if ( maxFlag == 1 )
	  if ( maxFlag == 1 || (shFl5 != 0 && shFl5 != 9))
	    colorstr = ";0:255:0";
	  else if ( maxFlag >= 2 && maxFlag <= 5 )
	    colorstr = ";255:175:0";
	  else if ( maxFlag >= 6 )
	    colorstr = ";255:0:0";

	  std::string valstr = boost::lexical_cast<std::string>(corr), synvalstr = valstr;
	  synvalstr += ";";
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
    pLetter.common = timeutil::to_iso_extended_string(time) + ",synop";
#ifdef METLIBS_BEFORE_4_9_5
    pLetter.data = std::vector<miutil::miString>(synopData.begin(), synopData.end());
#else
    pLetter.data = synopData;
#endif
    //TEST
    cerr << "HQC sender melding : " << pLetter.content() << endl;
    //TEST
    pluginB->sendMessage(pLetter);
  }
}


void HqcMainWindow::sendSelectedParam(const QString & param)
{
  LOG_SCOPE();

  std::string diParam = dianaName(param.latin1());
  if( diParam.empty() ) {
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
std::string HqcMainWindow::dianaName(const std::string& lbl) {
  NameMap::iterator dnit;
  for ( dnit = dnMap.begin(); dnit != dnMap.end(); dnit++ ) {
    if ( lbl == dnit.data() ) {
      return dnit.key();
    }
  }
  return "";
}

std::string HqcMainWindow::hqcType(int typeId, int env) {
  // Generates string to send to Diana
  std::string hqct = "none";
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
  ui->saveAction->setEnabled(enabled);
}

bool HqcMainWindow::isAlreadyStored(const timeutil::ptime& otime, int stnr) {
  for ( unsigned int i = 0; i < datalist->size(); i++) {
    if ( (*datalist)[i].otime() == otime && (*datalist)[i].stnr() == stnr )
      return true;
  }
  return false;
}

int HqcMainWindow::findTypeId(int typ, int pos, int par, const timeutil::ptime& oTime)
{
    int tpId = typ;
    mi_foreach_r(const kvalobs::kvObsPgm& op, obsPgmList) {
        if ( op.stationID() == pos && op.paramID() == par
                && timeutil::from_miTime(op.fromtime()) < oTime
                && timeutil::from_miTime(op.totime()) > oTime )
        {
            tpId = op.typeID();
            break;
        }
    }
    if( abs(tpId) > 503 ) {
        tpId = -32767;
        mi_foreach_r(const kvalobs::kvObsPgm& op, obsPgmList) {
            if(op.stationID() == pos && timeutil::from_miTime(op.fromtime()) < oTime) {
                const int opar = op.paramID();
                if((par == 105 && opar == 105)
                    || (par == 106 && opar == 105)
                    || (par == 109 && (opar == 104 || opar == 105 || opar == 106))
                    || (par == 110 && (opar == 104 || opar == 105 || opar == 106 || opar == 109))
                    || (par == 214 && opar == 213)
                    || (par == 216 && opar == 215)
                    || (par == 224 && opar == 223))
                {
                    tpId = -op.typeID();
                    break;
                }
            }
        }
    }
    return tpId;
}

void HqcMainWindow::makeTextDataList(kvservice::KvObsDataList& textDataList)
{
  //  cout << "textDataList.size = " << textDataList.size() << endl;
  for(kvservice::IKvObsDataList it=textDataList.begin(); it!=textDataList.end(); it++ ) {
    kvservice::KvObsData::kvTextDataList::iterator dit=it->textDataList().begin();
    while( dit != it->textDataList().end() ) {
      TxtDat txtd;
      txtd.stationId = dit->stationID();
      txtd.obstime   = timeutil::from_miTime(dit->obstime());
      txtd.original  = dit->original();
      txtd.paramId   = dit->paramID();
      txtd.tbtime    = timeutil::from_miTime(dit->tbtime());
      txtd.typeId    = dit->typeID();
      txtList.push_back(txtd);
      dit++;
    }
  }
}

void HqcMainWindow::makeObsDataList(kvservice::KvObsDataList& dataList)
{
    model::KvalobsData tdl;
    bool tdlUpd[NOPARAM];
    std::fill(tdlUpd, tdlUpd + NOPARAM, false);

    timeutil::ptime protime = timeutil::from_iso_extended_string("1800-01-01 00:00:00");
    int prtypeId = -1;
    int prstnr = 0;
    int aggPar = 0;
    int aggTyp = 0;
    int aggStat = 0;
    timeutil::ptime aggTime = timeutil::from_iso_extended_string("1800-01-01 00:00:00");

    for (kvservice::IKvObsDataList it = dataList.begin(); it != dataList.end(); it++) {

        kvservice::KvObsData::kvDataList::iterator dit = it->dataList().begin();
        //    IDataList dit = it->dataList().begin();
        int ditSize = it->dataList().size();
        int stnr = dit->stationID();
        int prParam = -1;
        int prSensor = -1;
        int ditNo = 0;
        while (dit != it->dataList().end()) {
            timeutil::ptime otime = timeutil::from_miTime(dit->obstime());
            timeutil::ptime tbtime = timeutil::from_miTime(dit->tbtime());
            const int d_param = dit->paramID(), d_type = dit->typeID(), d_sensor = dit->sensor(), d_sensor0 = d_sensor - '0';
            if (d_param < 0 or d_param >= NOPARAM) {
                std::cerr << "paramid out of range 0.." << NOPARAM << " for this observation:\n   " << *dit << endl;
                dit++;
                ditNo++;
                continue;
            }
            bool correctLevel = (dit->level() == HqcMainWindow::sLevel);
            bool correctTypeId;
            if (lstdlg->getSelectedStationTypes().contains("ALL") && d_sensor0 == 0) // FIXME sensor
                correctTypeId = true;
            else
                correctTypeId = typeIdFilter(stnr, d_type, d_sensor0, otime, d_param);
            //      if ( d_type < 0 && d_type != -342 ) {
            if (d_type < 0) {
                aggPar = d_param;
                aggTyp = d_type;
                aggTime = otime;
                aggStat = dit->stationID();
            } else {
                aggPar = 0;
                aggTyp = 0;
                aggStat = 0;
                aggTime = timeutil::from_iso_extended_string("1800-01-01 00:00:00");
            }

            int stnr = dit->stationID();
            int hour = otime.time_of_day().hours();
            int typeId = d_type;
            if ((otime == protime && stnr == prstnr && d_param == prParam && typeId == prtypeId && d_sensor == prSensor && lstdlg->showPrioritized())
                    || (!correctTypeId && !lstdlg->showPrioritized())) {
                protime = otime;
                prstnr = stnr;
                prtypeId = typeId;
                prSensor = d_sensor;
                prParam = -1;
                dit++;
                ditNo++;
                continue;
            }
            tdl.set_otime(otime);
            tdl.set_tbtime(tbtime);
            tdl.set_stnr(stnr);
            bool isaggreg = (stnr == aggStat && otime == aggTime && typeId == abs(aggTyp) && aggPar == d_param);

            if (correctTypeId && correctLevel && !isaggreg && !tdlUpd[d_param]) {
                tdl.set_typeId(d_param, typeId);
                tdl.set_showTypeId(typeId);
                tdl.set_orig(d_param, dit->original());
                tdl.set_corr(d_param, dit->corrected());
                tdl.set_sensor(d_param, d_sensor);
                tdl.set_level(d_param, dit->level());
                tdl.set_controlinfo(d_param, dit->controlinfo());
                tdl.set_useinfo(d_param, dit->useinfo());
                tdl.set_cfailed(d_param, dit->cfailed());
                if (d_type != 501)
                    tdlUpd[d_param] = true;
            }
            protime = otime;
            prstnr = stnr;
            prtypeId = typeId;
            prSensor = d_sensor;
            prParam = d_param;
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
            bool correctHqcType = hqcTypeFilter(tdl.showTypeId(), env, stnr); //  !!!

            ++dit;
            ++ditNo;
            if( dit == it->dataList().end() ) {
                otime = timeutil::from_iso_extended_string("1801-01-01 00:00:00");
                stnr = 0;
                typeId = 0;
            } else {
                otime = timeutil::from_miTime(dit->obstime());
                stnr = dit->stationID();
                typeId = dit->typeID();
            }
            bool errFl = false;
            if ((!correctHqcType || !correctLevel || !correctTypeId) && ditNo < ditSize - 1) {
                continue;
            } else if (ditNo != ditSize - 1) {
                for (int ip = 0; ip < NOPARAM; ip++) {
                    int shFl = tdl.flag(ip);
                    int shFl1 = shFl / 10000;
                    int shFl2 = shFl % 10000 / 1000;
                    int shFl3 = shFl % 1000 / 100;
                    int shFl4 = shFl % 100 / 10;
                    if (shFl1 > 1 || shFl2 > 1 || shFl3 > 1 || shFl4 > 1)
                        errFl = true;
                }
                if (!errFl && (lity == erLi || lity == erSa || lity == erLo)) {
                    continue;
                }
            }
            const bool timeFiltered = timeFilter(hour); // FIXME this is the hour from before ++dit, is this correct?
            if ((timeFiltered && !isAlreadyStored(protime, prstnr) && ((otime != protime || (otime == protime && stnr != prstnr))))
                    || (lstdlg->getSelectedStationTypes().contains("ALL") && typeId != prtypeId)) {
                datalist->push_back(tdl);
                tdl = model::KvalobsData();
                std::fill(tdlUpd, tdlUpd + NOPARAM, false);
            } else if (not timeFiltered) {
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

  settings.setValue("version", PVERSION);

  settings.beginWriteArray("t");
  for ( int hour = 0; hour < 24; hour++ ) {
    settings.setArrayIndex(hour);
    settings.setValue("t", clkdlg->hasHour(hour));
  }
  settings.endArray();

  {
      settings.beginGroup("lstdlg");
      const QStringList stationTypes = lstdlg->getSelectedStationTypes();
      settings.setValue("stationTypes", stationTypes);
      
      const QStringList counties = lstdlg->getSelectedCounties();
      settings.setValue("counties", counties);
      settings.endGroup();
  }

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
    LOG_SCOPE();
  QList<Param> params;

  QSettings settings("Meteorologisk Institutt", "Hqc");
  if ( !restoreGeometry(settings.value("geometry").toByteArray()) )
    cout << "CANNOT RESTORE GEOMETRY!!!!" << endl;

  QString savedVersion = settings.value("version", "??").toString();
  if( savedVersion != PVERSION ) {
      QMessageBox::information(this,
                               tr("HQC - Versjonsendring"),
                               tr("Du bruker en annen versjon av HQC enn sist (nå: %1, før: %2). "
                                  "Du må sjekke at innstillingene (valgte parametere, tispunkter, osv) er fortsatt korrekte.")
                               .arg(PVERSION).arg(savedVersion),
                               QMessageBox::Ok, QMessageBox::Ok);
  }

  settings.beginReadArray("t");
  for( int hour = 0; hour < 24; hour++ ) {
    settings.setArrayIndex(hour);
    clkdlg->setHour(hour, settings.value("t", true).toBool());
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

  {
      settings.beginGroup("lstdlg");
      QStringList stationTypesDefault;
      stationTypesDefault << "ALL";
      QStringList stationTypes = settings.value("stationTypes", stationTypesDefault).toStringList();
      lstdlg->setSelectedStationTypes(stationTypes);
      
      QStringList countiesDefault;
      countiesDefault << "ALL";
      QStringList counties = settings.value("counties", countiesDefault).toStringList();
      lstdlg->setSelectedCounties(counties);
      settings.endGroup();
  }
}
