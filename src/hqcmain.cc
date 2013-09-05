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
#include "DataList.hh"
#include "DataListModel.hh"
#include "dianashowdialog.h"
#include "discarddialog.h"
#include "errorlist.h"
#include "GetData.h"
#include "GetTextData.h"
#include "HintWidget.hh"
#include "hqc_paths.hh"
#include "hqc_utilities.hh"
#include "HqcDianaHelper.hh"
#include "HqcLogging.hh"
#include "KvalobsModelAccess.hh"
#include "KvMetaDataBuffer.hh"
#include "ListDialog.hh"
#include "MiDateTimeEdit.hh"
#include "mi_foreach.hh"
#include "QtKvalobsAccess.hh"
#include "rejectdialog.h"
#include "rejecttable.h"
#include "rejecttimeseriesdialog.h"
#include "textdatadialog.h"
#include "textdatatable.h"
#include "TimeseriesDialog.h"
#include "timeutil.hh"
#include "weatherdialog.h"

#include "StationDialog.hh"
#include "MainDialog.hh"

#ifndef slots
#define slots
#include <qUtilities/qtHelpDialog.h>
#include <qTimeseries/TSPlotDialog.h>
#undef slots
#else
#include <qUtilities/qtHelpDialog.h>
#include <qTimeseries/TSPlotDialog.h>
#endif
#include <qTimeseries/TSPlot.h>
#include <qUtilities/ClientButton.h>
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
#include <qdebug.h>

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

//#define NDEBUG
#include "debug.hh"

namespace {

const int VERSION_CHECK_TIMEOUT = 60*60*1000; // milliseconds

const char SETTING_HQC_GEOMETRY[] = "geometry";
const char SETTING_VERSION[] = "version";
const char SETTING_VERSION_FULL[] = "version_full";

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

    setIcon(QPixmap(hqc::getPath(hqc::IMAGEDIR)+"/hqc.png"));

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
    connect(ui->actionDianaConfig, SIGNAL(triggered()), dshdlg, SLOT(show()));
    connect(dshdlg, SIGNAL(dianaShowApply()), this, SLOT(dianaShowOK()));

    connect(ui->actionTextDataList, SIGNAL(triggered()), txtdlg, SLOT(show()));
    connect(txtdlg, SIGNAL(textDataApply()), SLOT(textDataOK()));

    connect(ui->actionRejectDecode, SIGNAL(triggered()), rejdlg, SLOT(show()));
    connect(rejdlg, SIGNAL(rejectApply()), SLOT(rejectedOK()));

    tsdlg = new TimeseriesDialog();
    tsdlg->hide();
    connect(ui->timeSeriesAction, SIGNAL(triggered()), tsdlg, SLOT(show()));
    connect(tsdlg, SIGNAL(TimeseriesApply()), SLOT(TimeseriesOK()));
    
    connect(this, SIGNAL(newStationList(std::vector<QString>&)),
            tsdlg, SLOT(newStationList(std::vector<QString>&)));
    connect(this, SIGNAL(newParameterList(const std::vector<int>&)),
            tsdlg, SLOT(newParameterList(const std::vector<int>&)));
    
    connect(ui->actionRejectSeries, SIGNAL(triggered()), rjtsdlg, SLOT(show()));
    connect(rjtsdlg, SIGNAL(tsRejectApply()), SLOT(rejectTimeseriesOK()));

    connect(ui->actionAcceptSeries, SIGNAL(triggered()), actsdlg, SLOT(show()));
    connect(actsdlg, SIGNAL(tsAcceptApply()), SLOT(acceptTimeseriesOK()));
    
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

    connect(ui->saveAction, SIGNAL(activated()), ui->treeErrors, SLOT(saveChanges()));
    connect(ui->treeErrors, SIGNAL(signalNavigateTo(const kvalobs::kvData&)),
            this, SLOT(navigateTo(const kvalobs::kvData&)));
    
    // make the timeseries-plot-dialog
    tspdialog = new TSPlotDialog(this);

    HelpDialog::Info info;
    info.path = (::hqc::getPath(::hqc::DOCDIR) + "/html").toStdString();

    HelpDialog::Info::Source helpsource;
    helpsource.source = "news.html";
    helpsource.name = "Kvhqc News";
    helpsource.defaultlink = "";
    info.src.push_back(helpsource);
    
    mHelpDialog = new HelpDialog(this, info);
    mHelpDialog->hide();
}

HqcMainWindow::~HqcMainWindow()
{
}

void HqcMainWindow::setReinserter(HqcReinserter* r, const QString& u)
{
    userName = u;
    reinserter = r;
    kda->setReinserter(reinserter);
}

void HqcMainWindow::startup()
{
    DisableGUI disableGUI(this);
    listExist = false;
    sLevel = 0;

    //-----------------------------------------------------------------

    readSettings();
    show();
    checkVersionSettings();
    qApp->processEvents();
    if (not reinserter) {
        mHints->addHint(tr("<h1>Authentication</h1>"
                           "You are not registered as operator! "
                           "You can see the data list, error log and error list, "
                           "but you cannot make changes in the kvalobs database!"));
    }

    // --- READ STATION INFO ----------------------------------------
    {
        BusyIndicator busy;
        statusBar()->message(tr("Reading station list..."));
        qApp->processEvents();
        readFromStation();
        statusBar()->message(tr("Reading parameter list..."));
        qApp->processEvents();
        readFromParam();
        qApp->processEvents();
    }

    statusBar()->message( tr("Welcome to kvhqc %1!").arg(PVERSION_FULL), 2000 );
    mVersionCheckTimer->start(VERSION_CHECK_TIMEOUT);
}

void HqcMainWindow::dianaShowOK()
{
    LOG_SCOPE("HqcMainWindow");
    mDianaHelper->updateDianaParameters();
    if (listExist)
        ListOK();
}

bool HqcMainWindow::saveDataToKvalobs(std::list<kvalobs::kvData>& toSave)
{
    LOG_SCOPE("HqcMainWindow");
#ifndef NDEBUG
    LOG4SCOPE_DEBUG(DBG1(toSave.size()));
    BOOST_FOREACH(const kvalobs::kvData& d, toSave)
        LOG4SCOPE_DEBUG(DBG1(d));
#endif

    if (not reinserter) {
        LOG4SCOPE_DEBUG("skipping data save, since user is not authenticated");
        return false;
    }
    BusyIndicator busy;
    const CKvalObs::CDataSource::Result_var result = reinserter->insert(toSave);
    if (result->res != CKvalObs::CDataSource::OK) {
        QMessageBox::critical(this, tr("Unable to insert"),
                              tr("An error occured when attempting to insert data into kvalobs. "
                                 "The message from kvalobs was\n%1").arg(QString(result->message)),
                              QMessageBox::Ok, QMessageBox::NoButton);
        return false;
    }
    return true;
}

bool HqcMainWindow::saveDataToKvalobs(kvalobs::kvData& toSave1)
{
    LOG_SCOPE("HqcMainWindow");
    LOG4SCOPE_DEBUG(DBG1(toSave1));

    if (not reinserter) {
        LOG4SCOPE_DEBUG("skipping data save, since user is not authenticated");
        return false;
    }
    BusyIndicator busy;
    const CKvalObs::CDataSource::Result_var result = reinserter->insert(toSave1);
    if (result->res != CKvalObs::CDataSource::OK) {
        QMessageBox::critical(this, tr("Unable to insert"),
                              tr("An error occured when attempting to insert data into kvalobs. "
                                 "The message from kvalobs was\n%1").arg(QString(result->message)),
                              QMessageBox::Ok, QMessageBox::NoButton);
        return false;
    }
    return true;
}

void HqcMainWindow::ListOK()
{
    LOG_SCOPE("HqcMainWindow");
    if (not mDianaHelper->isConnected()) {
        mHints->addHint(tr("<h1>Diana-Connection</h1>"
                           "No contact with diana! "
                           "You should connect to the command server via the button in the lower right in the hqc window, "
                           "and connect diana to the command server using the button in diana's window."));
    }
    const std::vector<int> selectedStations = lstdlg->getSelectedStations();
    if (selectedStations.empty()) {
        QMessageBox::warning(this,
                             tr("Station Selection"),
                             tr("No stations selected! At least one statione must be chosen."),
                             QMessageBox::Ok,
                             Qt::NoButton);
        return;
    }

    mSelectedTimes = lstdlg->getSelectedTimes();
    if (mSelectedTimes.empty()) {
        QMessageBox::warning(this,
                             tr("Time Selection"),
                             tr("No times selected! At least one time must be selected."),
                             QMessageBox::Ok,
                             Qt::NoButton);
        return;
    }

    mSelectedParameters = lstdlg->getSelectedParameters();
    if (mSelectedParameters.empty()) {
        QMessageBox::warning(this,
                             tr("Weather Element"),
                             tr("No weather element selected! At least one has to be chosen."),
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
    
    if (lity == daLi or lity == alLi or lity == alSa) {
        statusBar()->message(tr("Building data list..."));
        qApp->processEvents();

        DataListModel::Sensors_t sensors;
        BOOST_FOREACH(int stationId, selectedStations) {
            BOOST_FOREACH(int paramId, mSelectedParameters) {
                const KvMetaDataBuffer::ObsPgmList& opl = KvMetaDataBuffer::instance()->findObsPgm(stationId);
                Sensor sensor(stationId, paramId, 0, 0, 0);
                BOOST_FOREACH(const kvalobs::kvObsPgm op, opl) {
                    if (op.paramID() == paramId) {
                        sensor.typeId = op.typeID();
                        break;
                    }
                }
                if (sensor.typeId != 0) {
                    sensors.push_back(sensor);
                    LOG4SCOPE_DEBUG(DBG1(sensor.stationId) << DBG1(sensor.paramId) << DBG1(sensor.typeId));
                }
            }
        }

        EditAccessPtr eda = boost::make_shared<EditAccess>(kda);
        DataList* dl = new DataList(this);
        LOG4SCOPE_DEBUG(DBG1(stime) << DBG1(etime));
        dl->setSensorsAndTimes(eda, sensors, TimeRange(stime, etime));
        ui->ws->addSubWindow(dl);
    }

    if (lity == erLi or lity == erSa or lity == alLi or lity == alSa) {
        statusBar()->message(tr("Building error list..."));
        qApp->processEvents();
        ui->treeErrors->generateContents(mSelectedParameters, TimeRange(stime, etime),
                                         (lity == erSa or lity == alSa),
                                         datalist, modeldatalist);
    }

    tileHorizontal();

    std::vector<QString> stationList;
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
    LOG4SCOPE_DEBUG("newStationList emitted");

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
  std::vector<std::string> parameter;
  std::vector<POptions::PlotOptions> plotoptions;
  std::vector<int> parameterIndex;
  std::vector<int> stationIndex;

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
}

const listStat_l& HqcMainWindow::getStationDetails()
{
    BusyStatus busy(this, tr("Loading station info..."));
    return StationInfoBuffer::instance()->getStationDetails();
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

    GetTextData textDataReceiver;
    bool ok = false;
    try {
      ok = kvservice::KvApp::kvApp->getKvData(textDataReceiver, whichData);
    } catch (std::exception& e) {
      LOG4HQC_ERROR("KvMetaDataBuffer", "exception while retrieving text data: " << e.what());
    }
    if (not ok) {
      QMessageBox::critical(this, tr("No Textdata"), tr("Could not read text data."),
          QMessageBox::Ok, QMessageBox::NoButton);
      return;
    }
    new TextData(textDataReceiver.textData(), this);
}

void HqcMainWindow::rejectedOK()
{
    LOG_SCOPE("HqcMainWindow");
    CKvalObs::CService::RejectDecodeInfo rdInfo;
    rdInfo.fromTime = dateStr_( rejdlg->dtfrom );
    rdInfo.toTime = dateStr_( rejdlg->dtto );
    LOG4SCOPE_INFO(rdInfo.fromTime << " <-> " << rdInfo.toTime);
    kvservice::RejectDecodeIterator rdIt;
    bool ok = false;
    try {
      ok = kvservice::KvApp::kvApp->getKvRejectDecode(rdInfo, rdIt);
    } catch (std::exception& e) {
      LOG4HQC_ERROR("KvMetaDataBuffer", "exception while retrieving rejectdecode data: " << e.what());
    }
    if (not ok) {
      QMessageBox::critical(this, tr("No RejectDecode"), tr("Could not read rejectdecode."),
          QMessageBox::Ok, QMessageBox::NoButton);
      return;
    }

    std::string decoder = "comobs";
    kvalobs::kvRejectdecode reject;
    while (rdIt.next(reject)) {
        if (reject.decoder().substr(0, decoder.size()) != decoder)
            continue;
        if (reject.comment() == "No decoder for SMS code <12>!")
            continue;

        LOG4SCOPE_INFO(reject.tbtime() << ' ' << reject.message() << ' ' << reject.comment() << reject.decoder());
        rejList.push_back(reject);
    }
    new Rejects(rejList, this);
}

void HqcMainWindow::showWatchRR()
{
    Sensor sensor(83880, 110 /*kvalobs::PARAMID_RR_24*/, 0, 0, 302);
    const timeutil::ptime now = timeutil::now();
    timeutil::ptime tMiddle = now;

    const kvalobs::kvData data = ui->treeErrors->getKvData();
    if (data.paramID() == 110) {
        sensor.stationId = data.stationID();
        sensor.typeId = data.typeID();
        tMiddle = timeutil::from_miTime(data.obstime());
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

    mDianaHelper->setEnabled(false);
    EditAccessPtr eda = boost::make_shared<EditAccess>(kda);

    MainDialog main(eda, kma, sensor, time, this);
    if (main.exec()) {
        if (not eda->sendChangesToParent()) {
            QMessageBox::critical(this,
                                  tr("WatchRR"),
                                  tr("Sorry, your changes could not be saved and are lost!"),
                                  tr("OK"),
                                  "");
        } else {
            QMessageBox::information(this,
                                     tr("WatchRR"),
                                     tr("Your changes have been saved."),
                                     tr("OK"),
                                     "");
        }
    }
    mDianaHelper->setEnabled(true);
}

void HqcMainWindow::showWeather()
{
    const kvalobs::kvData data = ui->treeErrors->getKvData();

    Weather::WeatherDialog * wtd = Weather::WeatherDialog::getWeatherDialog(data, this, Qt::Window);
    if ( wtd ) {
        wtd->setReinserter( reinserter );
        wtd->show();
    }
}

void HqcMainWindow::errListMenu()
{
  listMenu(erLi);
}

void HqcMainWindow::errLogMenu()
{
  listMenu(erLo);
}

void HqcMainWindow::allListMenu()
{
  listMenu(alLi);
}

void HqcMainWindow::dataListMenu()
{
  listMenu(daLi);
}

void HqcMainWindow::errLisaMenu()
{
  listMenu(erSa);
}

void HqcMainWindow::allListSalenMenu()
{
    listMenu(alSa);
}

void HqcMainWindow::listMenu(listType lt)
{
    LOG_SCOPE("HqcMainWindow");
    lity = lt;

    QDateTime mx = QDateTime::currentDateTime();
    int noDays = lstdlg->getStart().daysTo(lstdlg->getEnd());
    if ( noDays < 27 ) {
        mx = mx.addSecs(-60*mx.time().minute());
        mx = mx.addSecs(3600);
        lstdlg->setEnd(mx);
    }
    lstdlg->show();
}

void HqcMainWindow::acceptTimeseriesOK()
{
#if 0
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
  std::vector<QString> chList;
  std::vector<double> newCorr;
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
  approveDialog->setWindowTitle(tr("%1 - Accepting Data").arg(QApplication::applicationName()));
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
#endif
}

void HqcMainWindow::rejectTimeseriesOK()
{
#if 0
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
  std::vector<QString> chList;
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
  discardDialog->setWindowTitle(tr("%1 - Rejecting Data").arg(QApplication::applicationName()));
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
#endif
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
    dialog->setWindowTitle(tr("Print screenshot"));
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
    LOG_SCOPE("HqcMainWindow");
    QFile versionFile(::hqc::getPath(::hqc::CONFDIR) + "/../hqc_current_version");
    if (versionFile.open(QIODevice::ReadOnly)) {
        QTextStream in(&versionFile);
        if (not in.atEnd()) {
            const long installedVersion = in.readLine().toLong();
            const long runningVersion = PVERSION_NUMBER_MAJOR_MINOR_PATCH;
            if( installedVersion > runningVersion ) {
                QMessageBox::information(this,
                                         tr("HQC - Update"),
                                         tr("The hqc-application has been updated on your computer. "
                                            "You should save any changes and start the hqc-application "
                                            "again to use the new version."),
                                         QMessageBox::Ok, Qt::NoButton);
            } else {
                //std::cout << "no update, now=" << runningVersion << " installed=" << installedVersion << std::endl;
            }
            mVersionCheckTimer->start(VERSION_CHECK_TIMEOUT);
            return;
        }
    }
    // something went wrong when reading the version info file
    LOG4SCOPE_WARN("error reading hqc_current_version, not renewing timer");
}

void HqcMainWindow::closeEvent(QCloseEvent* event)
{
  writeSettings();
  QWidget::closeEvent(event);
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

bool HqcMainWindow::typeIdFilter(int stnr, int typeId, int sensor, const timeutil::ptime& otime, int par)
{
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
    BOOST_FOREACH(const currentType& ct, currentTypeList) {
        if (stnr == ct.stnr &&
            abs(typeId) == ct.cTypeId &&
            sensor == ct.cSensor &&
            par == ct.par &&
            (ct.fDate.is_not_a_date() || otime.date() >= ct.fDate) &&
            (ct.tDate.is_not_a_date() || otime.date() <= ct.tDate))
        {
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

  bool ok = false;
  try {
    ok = kvservice::KvApp::kvApp->getKvData(dataReceiver, whichData);
  } catch (std::exception& e) {
    LOG4HQC_ERROR("KvMetaDataBuffer", "exception while retrieving rejectdecode data: " << e.what());
  }
  if (not ok) {
    QMessageBox::critical(this, tr("Data Retrieval"),
        tr("An error occured when attempting to retrieve data from kvalobs."),
        QMessageBox::Abort, QMessageBox::NoButton);
    return;
  }

  statusBar()->message(tr("Reading model data..."));
  qApp->processEvents();
  typedef std::list<kvalobs::kvModelData> ModelDataList;
  ModelDataList mdlist;
  modeldatalist.reserve(131072);
  modeldatalist.clear();

  ok = false;
  try {
    ok = kvservice::KvApp::kvApp->getKvModelData(mdlist, whichData);
  } catch (std::exception& e) {
    LOG4HQC_ERROR("KvMetaDataBuffer", "exception while retrieving model data: " << e.what());
  }
  if (not ok) {
    QMessageBox::critical(this, tr("Model Data Retrieval"),
        tr("An error occured when attempting to retrieve model data from kvalobs."),
        QMessageBox::Ignore, QMessageBox::NoButton);
  }
  modDatl mtdl;
  ModelDataList::const_iterator it=mdlist.begin();
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
    msg.setText(tr("The kvalobs databasen is not accessible."));
    msg.setInformativeText(tr("HQC terminates because it cannot be used without the kvalobs database."));
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
    LOG_SCOPE("HqcMainWindow");
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
				    int& env)
{
    try {
        const kvalobs::kvStation& station = KvMetaDataBuffer::instance()->findStation(stnr);
        name = QString(station.name().c_str());
        lat  = (station.lat());
        lon  = (station.lon());
        hoh  = (station.height());
        snr  = (station.wmonr());
        env  = (station.environmentid());
    } catch (std::runtime_error& e) {
        LOG4HQC_WARN("HqcMainWindow", "Error in station lookup: " << e.what());
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
    LOG_SCOPE("HqcMainWindow");
    mDianaHelper->setFirstObs();
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
    QMessageBox::about( this, tr("About Hqc"),
			tr("Hqc is a program for manual quality control of observations. "
                           "The program consists of editable tables with observations including "
                           "a time series diagram, and it can be connected to Diana."
                           "\n\n"
                           "The program is developed by "
                           "Knut Johansen, "
                           "Alexander Bürger, "
                           "Lisbeth Bergholt, "
                           "Vegard Bønes, "
                           "Audun Christoffersen at met.no.\n\n"
                           "You are using HQC version %1.").arg(PVERSION_FULL));
}

void HqcMainWindow::sendTimes()
{
    LOG_SCOPE("HqcMainWindow");
    
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
    LOG_SCOPE("HqcMainWindow");
    DBGV(kd);
    mDianaHelper->sendTime(kd.obstime());
    mDianaHelper->sendObservations(*datalist, modeldatalist, mSelectedParameters);
    mDianaHelper->sendStation(kd.stationID());
    mDianaHelper->sendSelectedParam(kd.paramID());

    int typeID = kd.typeID();
    if (typeID == -32767)
        typeID = 0;
    /*emit*/ statTimeReceived(kd.stationID(), kd.obstime(), typeID);

    ui->simpleCorrrections->navigateTo(Helpers::sensorTimeFromKvData(kd));
}

int HqcMainWindow::findTypeId(int tpId, int pos, int par, const timeutil::ptime& oTime)
{
    LOG_SCOPE("HqcMainWindow");
    const std::list<kvalobs::kvObsPgm>& obs_pgm = KvMetaDataBuffer::instance()->findObsPgm(pos);
    mi_foreach_r(const kvalobs::kvObsPgm& op, obs_pgm) {
        const timeutil::ptime ofrom = timeutil::from_miTime(op.fromtime());
        const timeutil::ptime oto   = timeutil::from_miTime(op.totime());
        if (op.paramID() == par and ofrom <= oTime and (oto.is_not_a_date_time() or oto >= oTime))
        {
            LOG4SCOPE_DEBUG("found obs_pgm " << op);
            tpId = op.typeID();
            break;
        }
    }
    if (abs(tpId) > 503) {
        LOG4SCOPE_DEBUG("typeid reset from " << tpId);
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
                    LOG4SCOPE_DEBUG(DBG1(tpId));
                    break;
                }
            }
        }
    }
    LOG4SCOPE_DEBUG("final" << DBG1(tpId));
    return tpId;
}

void HqcMainWindow::makeObsDataList(kvservice::KvObsDataList& dataList)
{
    if (dataList.empty() or dataList.begin()->dataList().empty())
        return;

    statusBar()->message(tr("Reading data for station %1").arg(dataList.begin()->dataList().begin()->stationID()));
    qApp->processEvents();

    const QSet<QString> selectedStationTypes = QSet<QString>::fromList(lstdlg->getSelectedStationTypes());
    const bool allStationTypes = lstdlg->isSelectAllStationTypes();
    const bool showPrioritized = lstdlg->showPrioritized();

    int prStation = -1;
    int env;

    BOOST_FOREACH(kvservice::KvObsData& od, dataList) {
        BOOST_FOREACH(const kvalobs::kvData& kvd, od.dataList()) {
            if (prStation != kvd.stationID()) {
                int snr;
                QString name;
                double lat, lon, hoh;
                findStationInfo(kvd.stationID(), name, lat, lon, hoh, snr, env);
                prStation = kvd.stationID();
            }

            if (not keepDataInList(kvd, abs(kvd.typeID()), env, selectedStationTypes, allStationTypes, showPrioritized))
               continue;

            putToDataList(kvd);
        }
    }
}

void HqcMainWindow::putToDataList(const kvalobs::kvData& kvd)
{
    const int absTypeID = /*std::abs*/(kvd.typeID());
    const model::KvalobsDataList::reverse_iterator rstop = datalist->rend();
    model::KvalobsDataList::reverse_iterator rit;
    for(rit = datalist->rbegin(); rit != rstop; ++rit) {
        const model::KvalobsData& inList = *rit;

        if (inList.otime() != kvd.obstime()) {
            // different obstime, stop searching and put new model::KvalobsData at end
            rit = rstop;
            break;
        }
        if (inList.stnr() == kvd.stationID() and inList.showTypeId() == absTypeID) {
            // found
            break;
        }
    }

    if (rit == rstop) {
        // did not find same station/|type|/sensor/level, put new model::KvalobsData at end

        int env, snr;
        QString name;
        double lat, lon, hoh;
        findStationInfo(kvd.stationID(), name, lat, lon, hoh, snr, env);

        model::KvalobsData tdl;
        tdl.set_stnr(kvd.stationID());
        tdl.set_name(name);
        tdl.set_latitude(lat);
        tdl.set_longitude(lon);
        tdl.set_altitude(hoh);
        tdl.set_snr(snr);
        tdl.set_showTypeId(absTypeID);
        tdl.set_otime(kvd.obstime());
        tdl.set_tbtime(kvd.tbtime()); // FIXME this assumes all parameters have the same tbtime

        datalist->push_back(tdl);
        rit = datalist->rbegin(); // new item at end
    }
    
    model::KvalobsData& tdl = *rit;
    const int pid = kvd.paramID();
    tdl.set_typeId     (pid, kvd.typeID());
    tdl.set_orig       (pid, kvd.original());
    tdl.set_corr       (pid, kvd.corrected());
    tdl.set_sensor     (pid, kvd.sensor() % '0');
    tdl.set_level      (pid, kvd.level());
    tdl.set_controlinfo(pid, kvd.controlinfo());
    tdl.set_useinfo    (pid, kvd.useinfo());
    tdl.set_cfailed    (pid, kvd.cfailed());
}

bool HqcMainWindow::keepDataInList(const kvalobs::kvData& kvd, int absTypeId, int env,
                                   const QSet<QString> selectedStationTypes,
                                   const bool allStationTypes, const bool showPrioritized)
{
    if (kvd.level() != HqcMainWindow::sLevel)
        return false;

    const int d_sensor0 = kvd.sensor() % '0';
    if (not (allStationTypes && d_sensor0 == 0)) // FIXME sensor
        if (not typeIdFilter(kvd.stationID(), kvd.typeID(), d_sensor0, kvd.obstime(), kvd.paramID()))
            return false;

    if (std::find(mSelectedParameters.begin(), mSelectedParameters.end(), kvd.paramID()) == mSelectedParameters.end())
        return false;

    if (not hqcTypeFilter(selectedStationTypes, absTypeId, env))
        return false;

    if (not timeFilter(kvd.obstime().time_of_day().hours()))
        return false;

    return true;
}

void HqcMainWindow::writeSettings()
{
    QList<Param> params;

    QSettings settings;
    settings.setValue(SETTING_HQC_GEOMETRY, saveGeometry());

    lstdlg->saveSettings(settings);
}

void HqcMainWindow::readSettings()
{
    LOG_SCOPE("HqcMainWindow");

    QSettings settings;
    if (not restoreGeometry(settings.value(SETTING_HQC_GEOMETRY).toByteArray()))
        LOG4SCOPE_WARN("cannot restore hqc main window geometry");

    lstdlg->restoreSettings(settings);
}

void HqcMainWindow::checkVersionSettings()
{
    QSettings settings;

    QString savedVersionFull = settings.value(SETTING_VERSION_FULL, "??").toString();
    if (savedVersionFull != PVERSION_FULL) {
        settings.setValue(SETTING_VERSION_FULL, PVERSION_FULL);
        helpNews();
    }

    QString savedVersion = settings.value(SETTING_VERSION, "??").toString();
    if (savedVersion != PVERSION) {
        settings.setValue(SETTING_VERSION, PVERSION);
        QMessageBox::information(this,
                                 tr("HQC - Version Change"),
                                 tr("You are using a different version of HQC than before (now: %1, before: %2). "
                                    "You have to check that all settings (chosen parameters, times, etc) are still correct.")
                                 .arg(PVERSION).arg(savedVersion),
                                 QMessageBox::Ok, QMessageBox::Ok);
    }
}

void HqcMainWindow::helpNews()
{
    mHelpDialog->showdoc(0, PVERSION_FULL);
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
