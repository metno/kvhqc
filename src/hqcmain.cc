/*
HQC - Free Software for Manual Quality Control of Meteorological Observations

Copyright (C) 2013 met.no

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
#include "config.h"
#include "dianashowdialog.h"
#include "discarddialog.h"
#include "errorlist.h"
#include "GetData.h"
#include "GetTextData.h"
#include "HintWidget.hh"
#include "identifyUser.h"
#include "hqc_paths.hh"
#include "hqc_utilities.hh"
#include "HqcDianaHelper.hh"
#include "KvalobsModelAccess.hh"
#include "KvalobsDataModel.h"
#include "KvalobsDataView.h"
#include "KvMetaDataBuffer.hh"
#include "ListDialog.hh"
#include "MiDateTimeEdit.hh"
#include "mi_foreach.hh"
#include "QtKvalobsAccess.hh"
#include "rejectdialog.h"
#include "rejecttable.h"
#include "rejecttimeseriesdialog.h"
#include "StationInformation.h"
#include "textdatadialog.h"
#include "TimeseriesDialog.h"
#include "timeutil.hh"
#include "weatherdialog.h"

#include "StationDialog.hh"
#include "MainDialog.hh"

#ifndef slots
#define slots
#include <qTimeseries/TSPlotDialog.h>
#undef slots
#else
#include <qTimeseries/TSPlotDialog.h>
#endif
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
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>

#include <algorithm>
#include <deque>
#include <functional>
#include <iomanip>
#include <stdexcept>

//#define NDEBUG
#include "debug.hh"

namespace {

const std::map<QString, QString> configNameToUserName = boost::assign::map_list_of
        ("[airpress]", "Lufttrykk")
        ("[temperature]", "Temperatur")
        ("[prec]", "Nedb�r")
        ("[visual]", "Visuell")
        ("[wave]", "Sj�gang")
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
  , reinserter(0)
  , dataModel(0)
  , sLevel(0)
  , listExist(false)
  , tsVisible(false)
  , ui(new Ui::HqcMainWindow)
  , mVersionCheckTimer(new QTimer(this))
  , mHints(new HintWidget(this))
  , kda(boost::make_shared<QtKvalobsAccess>())
  , kma(boost::make_shared<KvalobsModelAccess>())
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
    statusBar()->addPermanentWidget(pluginB, 0);


    // --- DEFINE DIALOGS --------------------------------------------
    const QString hqc_icon_path = ::hqc::getPath(::hqc::IMAGEDIR) + "/hqc.png";
    lstdlg = new ListDialog(this);
    lstdlg->setIcon( QPixmap(hqc_icon_path) );
    dshdlg = new DianaShowDialog(this);
    dshdlg->setIcon( QPixmap(hqc_icon_path) );
    txtdlg = new TextDataDialog(this);
    txtdlg->setIcon( QPixmap(hqc_icon_path) );
    rejdlg = new RejectDialog(this);
    rejdlg->setIcon( QPixmap(hqc_icon_path) );
    actsdlg = new AcceptTimeseriesDialog();
    actsdlg->hide();
    rjtsdlg = new RejectTimeseriesDialog();
    rjtsdlg->hide();
    
    // --- START -----------------------------------------------------
    rejdlg->hide();
    lstdlg->hide();

    mDianaHelper.reset(new HqcDianaHelper(dshdlg, pluginB));
    connect(mDianaHelper.get(), SIGNAL(receivedTime(const timeutil::ptime&)),
            this, SLOT(receivedTimeFromDiana(const timeutil::ptime&)));
    connect(mDianaHelper.get(), SIGNAL(receivedStation(int)),
            this, SLOT(receivedStationFromDiana(int)));

    connect(lstdlg, SIGNAL(ListApply()), this, SLOT(ListOK()));

    dshdlg->hide();
    connect( dshdlg, SIGNAL(dianaShowApply()), SLOT(dianaShowOK()));
    connect( dshdlg, SIGNAL(dianaShowHide()), SLOT(dianaShowMenu()));

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
    connect(this, SIGNAL(newParameterList(const std::vector<int>&)),
            tsdlg, SLOT(newParameterList(const std::vector<int>&)));
    
    connect(rjtsdlg, SIGNAL(tsRejectApply()), SLOT(rejectTimeseriesOK()));
    connect(rjtsdlg, SIGNAL(tsRejectHide()), SLOT(rejectTimeseries()));
    
    connect(actsdlg, SIGNAL(tsAcceptApply()), SLOT(acceptTimeseriesOK()));
    connect(actsdlg, SIGNAL(tsAcceptHide()), SLOT(acceptTimeseries()));
    
    connect(this,  SIGNAL(newStationList(std::vector<QString>&)),
            rjtsdlg, SLOT(newStationList(std::vector<QString>&)));
    connect(this,  SIGNAL(newParameterList(const std::vector<int>&)),
            rjtsdlg, SLOT(newParameterList(const std::vector<int>&)));
    
    connect(this,  SIGNAL(newStationList(std::vector<QString>&)),
            actsdlg, SLOT(newStationList(std::vector<QString>&)));
    connect(this,  SIGNAL(newParameterList(const std::vector<int>&)),
            actsdlg, SLOT(newParameterList(const std::vector<int>&)));
    
    connect(lstdlg, SIGNAL(fromTimeChanged(const QDateTime&)),
            tsdlg, SLOT(setFromTimeSlot(const QDateTime&)));
    
    connect(lstdlg, SIGNAL(toTimeChanged(const QDateTime&)),
            tsdlg, SLOT(setToTimeSlot(const QDateTime&)));
    
    // make the timeseries-plot-dialog
    tspdialog = new TSPlotDialog(this);
}

void HqcMainWindow::startup()
{
    DisableGUI disableGUI(this);
    listExist = false;
    sLevel = 0;
    tsVisible = false;

    // --- CHECK USER IDENTITY ----------------------------------------

    reinserter = Authentication::identifyUser(this, kvservice::KvApp::kvApp, "ldap-oslo.met.no", userName);
    kda->setReinserter(reinserter);

    //-----------------------------------------------------------------

    readSettings();
    show();
    qApp->processEvents();
    if (not reinserter) {
        mHints->addHint(tr("<h1>Autentisering</h1>"
                           "Du er ikke registrert som operat�r! "
                           "Du kan se dataliste, feillog og feilliste, "
                           "men ikke gj�re endringer i Kvalobsdatabasen!"));
    }

    // --- READ STATION INFO ----------------------------------------
    {
        BusyIndicator busy;
        statusBar()->message(tr("Leser stasjonsliste..."));
        qApp->processEvents();
        readFromStation();
        statusBar()->message(tr("Leser parameterliste..."));
        qApp->processEvents();
        readFromParam();
        qApp->processEvents();
    }

    statusBar()->message( tr("Velkommen til kvhqc %1!").arg(PVERSION_FULL), 2000 );
    mVersionCheckTimer->start(VERSION_CHECK_TIMEOUT);
}

void HqcMainWindow::dianaShowOK()
{
    LOG_SCOPE();
    mDianaHelper->updateDianaParameters();
    if (listExist)
        ListOK();
}

void HqcMainWindow::saveDataToKvalobs(const kvalobs::kvData & toSave)
{
    DBGV(toSave);
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
    if (not mDianaHelper->isConnected()) {
        mHints->addHint(tr("<h1>Dianaforbindelse</h1>"
                           "Har ikke kontakt med diana! "
                           "Du skulle kople til kommando-tjeneren via knappen underst til h�yre i hqc-vinduet, "
                           "og kople diana til tjeneren via knappen i diana sin vindu."));
    }
    const std::vector<int> selectedStations = lstdlg->getSelectedStations();
    if (selectedStations.empty()) {
        QMessageBox::warning(this,
                             tr("Stasjonsvalg"),
                             tr("Ingen stasjoner er valgt! Minst en stasjon m� velges"),
                             QMessageBox::Ok,
                             Qt::NoButton);
        return;
    }

    mSelectedTimes = lstdlg->getSelectedTimes();
    if (mSelectedTimes.empty()) {
        QMessageBox::warning(this,
                             tr("Tidspunktvalg"),
                             tr("Ingen tidspunkter er valgt! Minst ett tidspunkt m� velges"),
                             QMessageBox::Ok,
                             Qt::NoButton);
        return;
    }

    mSelectedParameters = lstdlg->getSelectedParameters();
    if (mSelectedParameters.empty()) {
        QMessageBox::warning(this,
                             tr("V�relement"),
                             tr("Ingen v�relement er valgt! V�relement m� velges"),
                             QMessageBox::Ok,
                             Qt::NoButton);
        return;
    }

    DisableGUI disableGUI(this);
    BusyIndicator busyIndicator;
    listExist = true;
    
    timeutil::ptime stime = timeutil::from_QDateTime(lstdlg->getStart());
    timeutil::ptime etime = timeutil::from_QDateTime(lstdlg->getEnd());
    
    readFromData(stime, etime, selectedStations);
    
  // All windows are shown later, in the tileHorizontal function
    
    if (lity == daLi or lity == alLi) {
        statusBar()->message(tr("Bygger dataliste..."));
        qApp->processEvents();

        model::KvalobsDataView * tableView = new model::KvalobsDataView(this);
        tableView->setAttribute(Qt::WA_DeleteOnClose);
        
        dataModel =
            new model::KvalobsDataModel(
                mSelectedParameters, datalist, modeldatalist,
                ui->stID->isChecked(), ui->poID->isChecked(), ui->heID->isChecked(),
#ifdef NDEBUG
                reinserter != 0,
#else
                true,
#endif
                tableView);
        
        connect(dataModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(TimeseriesOK()));
        connect(dataModel, SIGNAL(dataModification(const kvalobs::kvData &)), this, SLOT(saveDataToKvalobs(const kvalobs::kvData &)));
        
        // Functionality for hiding/showing rows in data list
        connect(ui->flID, SIGNAL(toggled(bool)), tableView, SLOT(toggleShowFlags(bool)));
        connect(ui->orID, SIGNAL(toggled(bool)), tableView, SLOT(toggleShowOriginal(bool)));
        connect(ui->moID, SIGNAL(toggled(bool)), tableView, SLOT(toggleShowModelData(bool)));

        connect(this, SIGNAL(statTimeReceived(int, const timeutil::ptime&, int)), tableView, SLOT(selectStation(int, const timeutil::ptime&, int)));
        connect(this, SIGNAL(timeReceived(const timeutil::ptime&)), tableView, SLOT(selectTime(const timeutil::ptime&)));

        connect(tableView, SIGNAL(signalNavigateTo(const kvalobs::kvData&)), this, SLOT(navigateTo(const kvalobs::kvData&)));

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
        statusBar()->message(tr("Bygger feilliste..."));
        qApp->processEvents();
        ErrorList * erl = new ErrorList(mSelectedParameters,
                                        stime,
                                        etime,
                                        this,
                                        lity,
                                        datalist,
                                        modeldatalist);
        connect(ui->saveAction, SIGNAL( activated() ), erl, SLOT( saveChanges() ) );
        connect(erl, SIGNAL(signalNavigateTo(const kvalobs::kvData&)),
                this, SLOT(navigateTo(const kvalobs::kvData&)));
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
    /*emit*/ newStationList(stationList);
    cerr << "newStationList emitted " << endl;


    //  send parameter names to ts dialog
    /*emit*/ newParameterList(mSelectedParameters);
    if ( lity != erLi && lity != erSa  ) {
        sendTimes();
    }

    statusBar()->message("");
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
      const std::list<kvalobs::kvParam>& plist = KvMetaDataBuffer::instance()->allParams();
      std::list<kvalobs::kvParam>::const_iterator it=plist.begin();
      BOOST_FOREACH(const kvalobs::kvParam& p, plist) {
          if (p.name() == parameter[ip]) {
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

bool HqcMainWindow::isShowTypeidInDataList() const
{
    return lstdlg->isSelectAllStationTypes();
}

const listStat_l& HqcMainWindow::getStationDetails()
{
    BusyStatus busy(this, tr("Loading station info..."));
    return StInfoSysBuffer::instance()->getStationDetails();
}

void HqcMainWindow::errListMenu() {
  lity = erLi;
  lstdlg->hide();
  listMenu();
}

void HqcMainWindow::errLogMenu() {
  lity = erLo;
  lstdlg->hide();
  listMenu();
}

void HqcMainWindow::allListMenu() {
  lity = alLi;
  lstdlg->hide();
  listMenu();
}
void HqcMainWindow::dataListMenu() {
  lity = daLi;
  lstdlg->hide();
  listMenu();
}

void HqcMainWindow::errLisaMenu() {
  lity = erSa;
  lstdlg->hide();
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

void HqcMainWindow::textDataOK()
{
    int stnr = txtdlg->stnr;
    QDate todt = (txtdlg->dtto).date();
    QTime toti = (txtdlg->dtto).time();
    timeutil::ptime dtto = timeutil::from_YMDhms(todt.year(), todt.month(), todt.day(), toti.hour(), 0, 0);
    QDate fromdt = (txtdlg->dtfrom).date();
    QTime fromti = (txtdlg->dtfrom).time();
    timeutil::ptime dtfrom = timeutil::from_YMDhms(fromdt.year(), fromdt.month(), fromdt.day(), fromti.hour(), 0, 0);

    kvservice::WhichDataHelper whichData;
    whichData.addStation(stnr, timeutil::to_miTime(dtfrom), timeutil::to_miTime(dtto));

    txtList.clear();
    GetTextData textDataReceiver(this);
    if(!kvservice::KvApp::kvApp->getKvData(textDataReceiver, whichData)){
        //cerr << "Finner ikke  textdatareceiver!!" << endl;
    }
    TextData* txtDat = new TextData(txtList);
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
    Sensor sensor(83880, 110 /*kvalobs::PARAMID_RR_24*/, 0, 0, 302);
    const timeutil::ptime now = timeutil::now();
    timeutil::ptime tMiddle = now;

    QMdiSubWindow* subWindow = ui->ws->activeSubWindow();
    if( subWindow ) {
        ErrorList* errorList = dynamic_cast<ErrorList*>(subWindow->widget());
        if (errorList) {
            const kvalobs::kvData data = errorList->getKvData();
            sensor.stationId = data.stationID();
            sensor.typeId = data.typeID();
            tMiddle = timeutil::from_miTime(data.obstime());
        }
    }

    timeutil::ptime timeTo = timeutil::ptime(tMiddle.date(), boost::posix_time::hours(6)) + boost::gregorian::days(7);
    timeutil::ptime timeFrom = timeTo - boost::gregorian::days(21);
    if (timeTo > now)
        timeTo = now;
    TimeRange time(timeFrom, timeTo);

    StationDialog sd(sensor, time);
    if (not sd.exec())
        return;

    sensor = sd.selectedSensor();
    time = sd.selectedTime();
    
    pluginB->connectToServer(); // disconnect
    EditAccessPtr eda = boost::make_shared<EditAccess>(kda);
    MainDialog main(eda, kma, sensor, time, this);
    if (main.exec()) {
        if (not eda->sendChangesToParent()) {
            QMessageBox::critical(0,
                                  tr("WatchRR"),
                                  tr("Sorry, your changes could not be saved and are lost!"),
                                  tr("OK"),
                                  "");
        } else {
            QMessageBox::information(0,
                                     tr("WatchRR"),
                                     tr("Your changes have been saved."),
                                     tr("OK"),
                                     "");
        }
    }
    pluginB->connectToServer(); // re-connect
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

    Weather::WeatherDialog * wtd = Weather::WeatherDialog::getWeatherDialog(data, this, Qt::Window);
    if ( wtd ) {
        wtd->setReinserter( reinserter );
        wtd->show();
    }
}

void HqcMainWindow::listMenu()
{
    LOG_SCOPE();

    QDateTime mx = QDateTime::currentDateTime();
    int noDays = lstdlg->getStart().daysTo(lstdlg->getEnd());
    if ( noDays < 27 ) {
        mx = mx.addSecs(-60*mx.time().minute());
        mx = mx.addSecs(3600);
        lstdlg->setEnd(mx);
    }
    lstdlg->show();
}

void HqcMainWindow::dianaShowMenu() {
  if ( dshdlg->isVisible() ) {
    dshdlg->hideAll();
  } else {
    dshdlg->showAll();
  }
}

void HqcMainWindow::timeseriesMenu() {
  if ( tsdlg->isVisible() ) {
    tsdlg->hideAll();
  } else {
    tsdlg->showAll();
  }
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
  bool maybeQC2;
  bool result = actsdlg->getResults(parameter,stime,etime,stationIndex, maybeQC2);
  if ( !result )
      return;
  long int stnr = stationIndex;
  const boost::posix_time::ptime ft = timeutil::from_QDateTime(stime);
  const boost::posix_time::ptime tt = timeutil::from_QDateTime(etime);
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
    QString stnr, parName = "???";
    stnr = stnr.setNum(dt.stationID());
    try {
        parName = QString::fromStdString(KvMetaDataBuffer::instance()->findParam(dt.paramID()).name());
    } catch(std::runtime_error&) {
    }
    ch = stnr + " " + QString::fromStdString(timeutil::to_iso_extended_string(timeutil::from_miTime(dt.obstime())))
        + ": " + parName + ": " + ori;
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
}

void HqcMainWindow::rejectTimeseriesOK() {
  QDateTime stime;
  QDateTime etime;
  QString parameter;
  int stationIndex;
  bool result = rjtsdlg->getResults(parameter,stime,etime,stationIndex);
  if ( !result )
      return;
  long int stnr = stationIndex;
  boost::posix_time::ptime ft = timeutil::from_iso_extended_string(stime.toString("yyyy-MM-dd hh:mm:ss").toStdString());
  boost::posix_time::ptime tt = timeutil::from_iso_extended_string(etime.toString("yyyy-MM-dd hh:mm:ss").toStdString());
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
    QString stnr, parName = "???";
    stnr = stnr.setNum(dt.stationID());
    try {
        parName = QString::fromStdString(KvMetaDataBuffer::instance()->findParam(dt.paramID()).name());
    } catch(std::runtime_error&) {
    }
    ch = stnr + " " + QString::fromStdString(timeutil::to_iso_extended_string(timeutil::from_miTime(dt.obstime())))
        + ": " + parName + ": " + cr;
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
                                         tr("Hqc-applikasjonen ble oppdatert p� din datamaskin. "
                                            "Du skulle lagre eventuelle endringer og starte "
                                            "hqc-applikasjonen p� nytt for � bruke den nye versjonen."),
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
    return mSelectedTimes.find(hour) != mSelectedTimes.end();
}

bool HqcMainWindow::hqcTypeFilter(const QSet<QString>& stationTypes, int typeId, int environment)
{
  if ( typeId == -1 || typeId == 501 ) return false;
  //  if ( typeId == -1 ) return false;
  if ( lstdlg->showSynop() or lstdlg->showPrioritized() ) return true;
  int atypeId = typeId < 0 ? -typeId : typeId;
  // FIXME this needs to match ListDialog.cc: StationTable::StationTable
  if (lstdlg->isSelectAllStationTypes()) return true;
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

  statusBar()->message(tr("Leser modelldata..."));
  qApp->processEvents();
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
    if (KvMetaDataBuffer::instance()->allStations().empty())
        exitNoKvalobs();
}

/*!
 Read the typeid file
*/
void HqcMainWindow::checkTypeId(int stnr)
{
    const std::list<kvalobs::kvObsPgm>& obs_pgm = KvMetaDataBuffer::instance()->findObsPgm(stnr);
    mi_foreach(const kvalobs::kvObsPgm& op, obs_pgm) {
        currentType crT;
        crT.stnr    = stnr;
        crT.par     = op.paramID();
        crT.fDate   = timeutil::from_miTime(op.fromtime()).date();
        crT.tDate   = timeutil::from_miTime(op.totime()).date();;
        crT.cLevel  = op.level();
        crT.cSensor = 0;
        crT.cTypeId = op.typeID();
        currentTypeList.push_back(crT);
    }
}

/*!
 Read the param table in the kvalobs database
*/
void HqcMainWindow::readFromParam()
{
    LOG_SCOPE();
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
    try {
        const kvalobs::kvStation& station = KvMetaDataBuffer::instance()->findStation(stnr);
        name = QString(station.name().c_str());
        lat  = (station.lat());
        lon  = (station.lon());
        hoh  = (station.height());
        snr  = (station.wmonr());
        env  = (station.environmentid());
    } catch (std::runtime_error& e) {
        std::cerr << "Error in station lookup: " << e.what() << std::endl;
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
    mDianaHelper->setFirstObs();
    cerr << "HqcMainWindow::closeWindow()\n";
    ui->ws->closeActiveSubWindow();
    /*emit*/ windowClose();
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
                           "Programmet best�r av editerbare tabeller med observasjoner samt"
                           "tidsseriediagram, og har forbindelse med Diana."
                           "\n\n"
                           "Programmet utvikles av "
                           "Knut Johansen, "
                           "Alexander B�rger, "
                           "Lisbeth Bergholt, "
                           "Vegard B�nes, "
                           "Audun Christoffersen i met.no.\n\n"
                           "Du bruker HQC versjon %1.").arg(PVERSION_FULL));
}

void HqcMainWindow::sendTimes()
{
    LOG_SCOPE();
    
    std::set<timeutil::ptime> allTimes;
    BOOST_FOREACH(const model::KvalobsData& d, *datalist)
        allTimes.insert(d.otime());
    mDianaHelper->sendTimes(allTimes);
}

void HqcMainWindow::receivedStationFromDiana(int stationid)
{
    /*emit*/ statTimeReceived(stationid, mDianaHelper->dianaTime(), 0);
}

void HqcMainWindow::receivedTimeFromDiana(const timeutil::ptime& time)
{
    /*emit*/ timeReceived(time);
    mDianaHelper->sendObservations(*datalist, modeldatalist, mSelectedParameters);
}

void HqcMainWindow::aboutQt()
{
    QMessageBox::aboutQt(this);
}

void HqcMainWindow::navigateTo(const kvalobs::kvData& kd)
{
    LOG_SCOPE();
    DBGV(kd);
    mDianaHelper->sendTime(kd.obstime());
    mDianaHelper->sendObservations(*datalist, modeldatalist, mSelectedParameters);
    mDianaHelper->sendStation(kd.stationID());
    mDianaHelper->sendSelectedParam(kd.paramID());

    int typeID = kd.typeID();
    if (typeID == -32767)
        typeID = 0;
    /*emit*/ statTimeReceived(kd.stationID(), kd.obstime(), typeID);
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
    const std::list<kvalobs::kvObsPgm>& obs_pgm = KvMetaDataBuffer::instance()->findObsPgm(pos);
    mi_foreach_r(const kvalobs::kvObsPgm& op, obs_pgm) {
        const timeutil::ptime ofrom = timeutil::from_miTime(op.fromtime());
        const timeutil::ptime oto   = timeutil::from_miTime(op.totime());
        if (op.paramID() == par and ofrom <= oTime and (oto.is_not_a_date_time() or oto >= oTime))
        {
            tpId = op.typeID();
            break;
        }
    }
    if( abs(tpId) > 503 ) {
        tpId = -32767;
        mi_foreach_r(const kvalobs::kvObsPgm& op, obs_pgm) {
            if (timeutil::from_miTime(op.fromtime()) < oTime) {
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
    if (dataList.empty() or dataList.begin()->dataList().empty())
        return;

    statusBar()->message(tr("Leser data for stasjon %1").arg(dataList.begin()->dataList().begin()->stationID()));
    qApp->processEvents();

    model::KvalobsData tdl;
    bool tdlUpd[NOPARAM];
    std::fill(tdlUpd, tdlUpd + NOPARAM, false);

    const QSet<QString> selectedStationTypes = QSet<QString>::fromList(lstdlg->getSelectedStationTypes());
    const bool allStationTypes = lstdlg->isSelectAllStationTypes();
    const bool showPrioritized = lstdlg->showPrioritized();
    const timeutil::ptime badtime = timeutil::from_iso_extended_string("1800-01-01 00:00:00");

    timeutil::ptime protime = badtime, aggTime = badtime;
    int prtypeId = -1;
    int prstnr = 0;
    int aggPar = 0;
    int aggTyp = 0;
    int aggStat = 0;

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
            if (allStationTypes && d_sensor0 == 0) // FIXME sensor
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
                aggTime = badtime;
            }

            int stnr = dit->stationID();
            int hour = otime.time_of_day().hours();
            int typeId = d_type;
            if ((otime == protime && stnr == prstnr && d_param == prParam && typeId == prtypeId && d_sensor == prSensor && showPrioritized)
                    || (!correctTypeId && not showPrioritized)) {
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
                tdl.set_sensor(d_param, d_sensor % '0');
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
            const bool correctHqcType = hqcTypeFilter(selectedStationTypes, tdl.showTypeId(), env);

            ++dit;
            ++ditNo;
            if( dit == it->dataList().end() ) {
                otime = badtime;
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
                    || (allStationTypes && typeId != prtypeId)) {
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

    QSettings settings;
    settings.setValue("geometry", saveGeometry());

    settings.setValue("version", PVERSION);

    lstdlg->saveSettings(settings);
}

void HqcMainWindow::readSettings()
{
    LOG_SCOPE();

    QSettings settings;
    if (not restoreGeometry(settings.value("geometry").toByteArray()))
        cout << "CANNOT RESTORE GEOMETRY!!!!" << endl;

    QString savedVersion = settings.value("version", "??").toString();
    if (savedVersion != PVERSION) {
        QMessageBox::information(this,
                                 tr("HQC - Versjonsendring"),
                                 tr("Du bruker en annen versjon av HQC enn sist (n�: %1, f�r: %2). "
                                    "Du m� sjekke at innstillingene (valgte parametere, tispunkter, osv) er fortsatt korrekte.")
                                 .arg(PVERSION).arg(savedVersion),
                                 QMessageBox::Ok, QMessageBox::Ok);
    }

    lstdlg->restoreSettings(settings);
}

void HqcMainWindow::moveEvent(QMoveEvent* event)
{
    QMainWindow::moveEvent(event);
    mHints->updatePosition();
}

void HqcMainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    mHints->updatePosition();
}
