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
#include "AutoColumnView.hh"
#include "BusyIndicator.h"
#include "config.h"
#include "DataList.hh"
#include "DataView.hh"
#include "dianashowdialog.h"
#include "discarddialog.h"
#include "errorlist.h"
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

#include <kvalobs/kvData.h>

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

#include <QtCore/qfile.h>
#include <QtCore/qsettings.h>
#include <QtCore/QTextStream>
#include <QtCore/qtimer.h>
#include <QtCore/qurl.h>
#include <QtGui/QDesktopServices>
#include <QtGui/QMdiSubWindow>
#include <QtGui/QMessageBox>
#include <QtGui/QPixmap>
#include <QtGui/QPrinter>
#include <QtGui/QPrintDialog>

#include <boost/bind.hpp>
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
  , reinserter(0)
  , listExist(false)
  , ui(new Ui::HqcMainWindow)
  , mAutoColumnView(new AutoColumnView)
  , mAutoDataList(new DataList(this))
  , mVersionCheckTimer(new QTimer(this))
  , mHints(new HintWidget(this))
  , kda(boost::make_shared<QtKvalobsAccess>())
  , kma(boost::make_shared<KvalobsModelAccess>())
  , eda(boost::make_shared<EditAccess>(kda))
{
    ui->setupUi(this);
    ui->treeErrors->setDataAccess(eda, kma);
    ui->simpleCorrrections->setDataAccess(eda, kma);

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
    mDianaHelper->setDataAccess(eda, kma);

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

    mDianaHelper  ->signalNavigateTo.connect(boost::bind(&HqcMainWindow::navigateTo, this, _1));
    ui->treeErrors->signalNavigateTo.connect(boost::bind(&HqcMainWindow::navigateTo, this, _1));

    mAutoDataList->setDataAccess(eda, kma);
    QMdiSubWindow* adlsw = ui->ws->addSubWindow(mAutoDataList);
    adlsw->setWindowTitle(tr("Automatic Data List"));
    mAutoColumnView->attachView(mAutoDataList);

    eda->obsDataChanged.connect(boost::bind(&HqcMainWindow::onDataChanged, this, _1, _2));
    ui->saveAction->setEnabled(false); // no changes yet
    
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

    tileHorizontal();
}

HqcMainWindow::~HqcMainWindow()
{
    mDianaHelper  ->signalNavigateTo.disconnect(boost::bind(&HqcMainWindow::navigateTo, this, _1));
    ui->treeErrors->signalNavigateTo.disconnect(boost::bind(&HqcMainWindow::navigateTo, this, _1));
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

void HqcMainWindow::ListOK()
{
    LOG_SCOPE("HqcMainWindow");
    if (not mDianaHelper->isConnected()) {
        mHints->addHint(tr("<h1>Diana-Connection</h1>"
                           "No contact with diana! "
                           "You should connect to the command server via the button in the lower right in the hqc window, "
                           "and connect diana to the command server using the button in diana's window."));
    }
    // FIXME pack selectedStations, selectedTimes, ... in class, psas this to AnalyseErrors
    const std::vector<int> selectedStations = lstdlg->getSelectedStations();
    if (selectedStations.empty()) {
        QMessageBox::warning(this,
                             tr("Station Selection"),
                             tr("No stations selected! At least one statione must be chosen."),
                             QMessageBox::Ok,
                             Qt::NoButton);
        return;
    }

    std::vector<int> mSelectedParameters;
    std::set<int> mSelectedTimes;
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
    
    // All windows are shown later, in the tileHorizontal function
    
    DataView::Sensors_t sensors;
    BOOST_FOREACH(int stationId, selectedStations) {
        BOOST_FOREACH(int paramId, mSelectedParameters) {
            const KvMetaDataBuffer::ObsPgmList& opl = KvMetaDataBuffer::instance()->findObsPgm(stationId);
            Sensor sensor(stationId, paramId, 0, 0, 0);
            BOOST_FOREACH(const kvalobs::kvObsPgm op, opl) {
                if (op.paramID() == paramId) {
                    sensor.typeId = op.typeID(); // FIXME choose the correct typeId here; what about negative typeId's?
                    break;
                }
            }
            if (sensor.typeId != 0) {
                sensors.push_back(sensor);
                LOG4SCOPE_DEBUG(DBG1(sensor));
            }
        }
    }
    LOG4SCOPE_DEBUG(DBG1(stime) << DBG1(etime));
    const TimeRange timeLimits(stime, etime);

    if (lity == daLi or lity == alLi or lity == alSa) {
        BusyStatus busyData(this, tr("Building data list..."));

        DataList* dl = new DataList(this);
        dl->setDataAccess(eda, kma);
        dl->setSensorsAndTimes(sensors, timeLimits);
        dl->signalNavigateTo.connect(boost::bind(&HqcMainWindow::navigateTo, this, _1));
        QMdiSubWindow* adlsw = ui->ws->addSubWindow(dl);
        adlsw->setWindowTitle(tr("Data List for Selected Stations/Parameters/Time"));
    }

    if (lity == erLi or lity == erSa or lity == alLi or lity == alSa) {
        BusyStatus busyErrors(this, tr("Building error list..."));

        ui->treeErrors->setErrorsForSalen(lity == erSa or lity == alSa);
        ui->treeErrors->setSensorsAndTimes(sensors, timeLimits);
    }

    tileHorizontal();

    std::vector<QString> stationList;
    BOOST_FOREACH(int stnr, selectedStations) {
        QString name;
        double lat,lon,hoh;
        int env;
        int snr;
        findStationInfo(stnr,name,lat,lon,hoh,snr,env);
        const QString statId = QString::number(stnr) + " " + name;
        stationList.push_back(statId);
    }
    /*emit*/ newStationList(stationList);
    LOG4SCOPE_DEBUG("newStationList emitted");

    //  send parameter names to ts dialog
    /*emit*/ newParameterList(mSelectedParameters);
    if (lity != erLi && lity != erSa) {
        mDianaHelper->setSensorsAndTimes(sensors, timeLimits);
    }

    statusBar()->message("");
}

void HqcMainWindow::TimeseriesOK()
{
    LOG4HQC_ERROR("HqcMainWindow", "FIXME implement this in TimeSeriesView");
}

inline QString dateStr_( const QDateTime & dt )
{
  QString ret = dt.toString( Qt::ISODate );
  ret[ 10 ] = ' ';
  return ret;
}

void HqcMainWindow::textDataOK()
{
    const timeutil::ptime dtto = timeutil::from_QDateTime(timeutil::clearedMinutesAndSeconds(txtdlg->dtto));
    const timeutil::ptime dtfrom = timeutil::from_QDateTime(timeutil::clearedMinutesAndSeconds(txtdlg->dtfrom));
    TextData::showTextData(txtdlg->stnr, TimeRange(dtfrom, dtto), this);
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
    std::vector<kvalobs::kvRejectdecode> rejList;
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

    EditDataPtr obs = ui->treeErrors->getObs();
    if (obs) {
        const SensorTime& st = obs->sensorTime();
        if (st.sensor.paramId == 110)
            sensor = st.sensor;
        tMiddle = timeutil::from_miTime(st.time);
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
    EditDataPtr obs = ui->treeErrors->getObs();
    if (obs) {
        LOG4HQC_DEBUG("HqcMainWindow", "sorry, no weather dialog for " << obs->sensorTime());
//         Weather::WeatherDialog * wtd = Weather::WeatherDialog::getWeatherDialog(data, this, Qt::Window);
//         if ( wtd ) {
//             wtd->setReinserter( reinserter );
//             wtd->show();
//         }
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

void HqcMainWindow::aboutQt()
{
    QMessageBox::aboutQt(this);
}

void HqcMainWindow::onUndoChanges()
{
    eda->popUpdate();
}

void HqcMainWindow::onSaveChanges()
{
    if (not eda->sendChangesToParent()) {
        QMessageBox::critical(this, tr("HQC - Saving data"),
                              tr("Sorry, your changes could not be saved!"),
                              tr("OK"),
                              "");
    } else {
        QMessageBox::information(this,
                                 tr("HQC - Saving data"),
                                 tr("Your changes have been saved."),
                                 tr("OK"),
                                 "");
    }
}

void HqcMainWindow::navigateTo(const SensorTime& st)
{
    LOG_SCOPE("HqcMainWindow");
    LOG4SCOPE_DEBUG(DBG1(st));

    mDianaHelper->navigateTo(st);
    ui->simpleCorrrections->navigateTo(st);
    mAutoColumnView->navigateTo(st);
    BOOST_FOREACH(QMdiSubWindow* sw, ui->ws->subWindowList()) {
        if (DataList* dl = dynamic_cast<DataList*>(sw->widget())) {
            if (dl != mAutoDataList)
                dl->navigateTo(st);
        }
    }
}

void HqcMainWindow::onDataChanged(ObsAccess::ObsDataChange what, ObsDataPtr obs)
{
    LOG4HQC_DEBUG("HqcMainWindow", DBG1(eda->countU()));
    ui->saveAction->setEnabled(eda->countU() > 0);
}


void HqcMainWindow::writeSettings()
{
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
