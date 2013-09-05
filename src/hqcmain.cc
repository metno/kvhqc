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

#include "AutoColumnView.hh"
#include "BusyIndicator.h"
#include "config.h"
#include "DataList.hh"
#include "DataView.hh"
#include "dianashowdialog.h"
#include "EditVersionModel.hh"
#include "errorlist.h"
#include "ExtremesTableModel.hh"
#include "ExtremesView.hh"
#include "FindExtremeValues.hh"
#include "HintWidget.hh"
#include "hqc_paths.hh"
#include "hqc_utilities.hh"
#include "HqcDianaHelper.hh"
#include "KvalobsModelAccess.hh"
#include "KvMetaDataBuffer.hh"
#include "ListDialog.hh"
#include "QtKvalobsAccess.hh"
#include "QNoCloseMdiSubWindow.hh"
#include "rejectdialog.h"
#include "rejecttable.h"
#include "textdatadialog.h"
#include "textdatatable.h"
#include "TimeSeriesView.hh"
#include "timeutil.hh"
#include "WeatherDialog.hh"

#include "StationDialog.hh"
#include "MainDialog.hh"

#include <kvalobs/kvData.h>

#ifndef slots
#define slots
#include <qUtilities/qtHelpDialog.h>
#undef slots
#else
#include <qUtilities/qtHelpDialog.h>
#endif
#include <qUtilities/ClientButton.h>

#include <QtCore/qfile.h>
#include <QtCore/qsettings.h>
#include <QtCore/QTextStream>
#include <QtCore/qtimer.h>
#include <QtCore/qurl.h>
#include <QtGui/QDesktopServices>
#include <QtGui/QMdiSubWindow>
#include <QtGui/QMessageBox>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtGui/QPrinter>
#include <QtGui/QPrintDialog>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

#define MILOGGER_CATEGORY "kvhqc.HqcMainWindow"
#include "HqcLogging.hh"

namespace {

const int VERSION_CHECK_TIMEOUT = 60*60*1000; // milliseconds

const char SETTING_HQC_GEOMETRY[] = "geometry";
const char SETTING_VERSION[] = "version";
const char SETTING_VERSION_FULL[] = "version_full";

#include "../src/icon_redo.xpm"
#include "../src/icon_undo.xpm"

} // anonymous namespace

HqcMainWindow::HqcMainWindow()
  : QMainWindow( 0, tr("HQC"))
  , reinserter(0)
  , listExist(false)
  , ui(new Ui::HqcMainWindow)
  , mVersionCheckTimer(new QTimer(this))
  , mHints(new HintWidget(this))
  , kda(boost::make_shared<QtKvalobsAccess>())
  , kma(boost::make_shared<KvalobsModelAccess>())
  , eda(boost::make_shared<EditAccess>(kda))
  , mEditVersions(new EditVersionModel(eda))
  , mTimeSeriesView(new TimeSeriesView(this))
  , mAutoColumnView(new AutoColumnView)
  , mAutoDataList(new DataList(this))
  , mExtremesView(new ExtremesView())
{
    ui->setupUi(this);
    ui->treeErrors->setDataAccess(eda, kma);
    ui->simpleCorrrections->setDataAccess(eda, kma);
    ui->treeChanges->setModel(mEditVersions.get());

    QIcon iconRedo, iconUndo;
    iconRedo.addPixmap(QPixmap(icon_redo));
    iconUndo.addPixmap(QPixmap(icon_undo));
    ui->actionRedo->setIcon(iconRedo);
    ui->actionUndo->setIcon(iconUndo);

    connect(mVersionCheckTimer, SIGNAL(timeout()), this, SLOT(onVersionCheckTimeout()));

    mVersionCheckTimer->setSingleShot(true);

    setIcon(QPixmap(hqc::getPath(hqc::IMAGEDIR)+"/hqc.png"));

    QPixmap icon_listdlg( ::hqc::getPath(::hqc::IMAGEDIR) + "/table.png");
    ui->dataListAction->setIcon(icon_listdlg);

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
    
    // --- START -----------------------------------------------------
    rejdlg->hide();
    lstdlg->hide();

    mDianaHelper.reset(new HqcDianaHelper(dshdlg, pluginB));
    mDianaHelper->setDataAccess(eda, kma);

    mTimeSeriesView->setDataAccess(eda, kma);

    connect(lstdlg, SIGNAL(ListApply()), this, SLOT(ListOK()));

    dshdlg->hide();
    connect(ui->actionDianaConfig, SIGNAL(triggered()), dshdlg, SLOT(show()));
    connect(dshdlg, SIGNAL(dianaShowApply()), this, SLOT(dianaShowOK()));

    connect(ui->actionTextDataList, SIGNAL(triggered()), txtdlg, SLOT(show()));
    connect(txtdlg, SIGNAL(textDataApply()), SLOT(textDataOK()));

    connect(ui->actionRejectDecode, SIGNAL(triggered()), rejdlg, SLOT(show()));
    connect(rejdlg, SIGNAL(rejectApply()), SLOT(rejectedOK()));

    mDianaHelper  ->signalNavigateTo.connect(boost::bind(&HqcMainWindow::navigateTo, this, _1));
    ui->treeErrors->signalNavigateTo.connect(boost::bind(&HqcMainWindow::navigateTo, this, _1));

    mAutoDataList->setDataAccess(eda, kma);
    QNoCloseMdiSubWindow* adlsw = new QNoCloseMdiSubWindow(ui->ws);
    adlsw->setWidget(mAutoDataList);
    adlsw->setWindowTitle(tr("Automatic Data List"));
    ui->ws->addSubWindow(adlsw);
    mAutoColumnView->attachView(mAutoDataList);

    QNoCloseMdiSubWindow* tssw = new QNoCloseMdiSubWindow(ui->ws);
    tssw->setWidget(mTimeSeriesView);
    tssw->setWindowTitle(tr("Time Series"));
    ui->ws->addSubWindow(tssw);
    mAutoColumnView->attachView(mTimeSeriesView);

    mExtremesView->setVisible(false);
    mExtremesView->signalNavigateTo.connect(boost::bind(&HqcMainWindow::navigateTo, this, _1));

    eda->obsDataChanged.connect(boost::bind(&HqcMainWindow::onDataChanged, this, _1, _2));
    ui->saveAction->setEnabled(false); // no changes yet
    
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

void HqcMainWindow::startup(const QString& captionSuffix)
{
    setCaption("HQC " + captionSuffix);
    
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
    METLIBS_LOG_SCOPE();
    mDianaHelper->updateDianaParameters();
    if (listExist)
        ListOK();
}

void HqcMainWindow::ListOK()
{
    METLIBS_LOG_SCOPE();
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

    const TimeRange timeLimits = lstdlg->getTimeRange();

    DataView::Sensors_t sensors;
    BOOST_FOREACH(int stationId, selectedStations) {
        BOOST_FOREACH(int paramId, mSelectedParameters) {
            const KvMetaDataBuffer::ObsPgmList& opl = KvMetaDataBuffer::instance()->findObsPgm(stationId);
            Sensor sensor(stationId, paramId, 0, 0, 0);
            BOOST_FOREACH(const kvalobs::kvObsPgm op, opl) {
              const int p = op.paramID();
              if (p == paramId) {
                sensor.typeId = op.typeID();
                sensors.push_back(sensor);
              }
              if (Helpers::aggregatedParameter(p, paramId)) {
                sensor.typeId = -op.typeID();
                sensors.push_back(sensor);
              }
            }
        }
    }

    if (lity == daLi or lity == alLi or lity == alSa) {
        BusyStatus busyData(this, tr("Building data list..."));

        DataList* dl = new DataList(this);
        dl->setDataAccess(eda, kma);
        dl->setSensorsAndTimes(sensors, timeLimits);
        dl->signalNavigateTo.connect(boost::bind(&HqcMainWindow::navigateTo, this, _1));
        QMdiSubWindow* adlsw = ui->ws->addSubWindow(dl);
        adlsw->setWindowTitle(tr("Data List for Selected Stations/Parameters/Time"));
        adlsw->setVisible(true);
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
    METLIBS_LOG_DEBUG("newStationList emitted");

    //  send parameter names to ts dialog
    /*emit*/ newParameterList(mSelectedParameters);
    if (lity != erLi && lity != erSa) {
        mDianaHelper->setSensorsAndTimes(sensors, timeLimits);
    }

    statusBar()->message("");
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
    METLIBS_LOG_SCOPE();
    CKvalObs::CService::RejectDecodeInfo rdInfo;
    rdInfo.fromTime = dateStr_( rejdlg->dtfrom );
    rdInfo.toTime = dateStr_( rejdlg->dtto );
    METLIBS_LOG_INFO(rdInfo.fromTime << " <-> " << rdInfo.toTime);
    kvservice::RejectDecodeIterator rdIt;
    bool ok = false;
    try {
      ok = kvservice::KvApp::kvApp->getKvRejectDecode(rdInfo, rdIt);
    } catch (std::exception& e) {
      METLIBS_LOG_ERROR("exception while retrieving rejectdecode data: " << e.what());
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

        METLIBS_LOG_INFO(reject.tbtime() << ' ' << reject.message() << ' ' << reject.comment() << reject.decoder());
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
    EditAccessPtr eda2 = boost::make_shared<EditAccess>(eda);
    MainDialog main(eda2, kma, sensor, time, this);
    if (main.exec()) {
      eda->newVersion();
      if (not eda2->sendChangesToParent(false)) {
        QMessageBox::critical(this,
            tr("WatchRR"),
            tr("Sorry, your changes could not be saved and are lost!"),
            tr("OK"),
            "");
      }
    }
    mDianaHelper->setEnabled(true);
}

void HqcMainWindow::showWeather()
{
  Sensor s(10380, 211, 0, 0, 311);
  const timeutil::ptime now = timeutil::now();
  const timeutil::ptime roundedNow = timeutil::ptime(now.date(), boost::posix_time::time_duration(now.time_of_day().hours(),0,0))
      + boost::posix_time::hours(1);
  timeutil::ptime t(roundedNow); //(boost::gregorian::day_clock::universal_day(),  boost::posix_time::time_duration(0,0,0)); 

  EditDataPtr obs = ui->treeErrors->getObs();
  if (obs) {
    const SensorTime& st = obs->sensorTime();
    s = st.sensor;
    t = st.time;
  }
  
  const TimeRange limits(t + boost::gregorian::days(-7), std::min(t + boost::gregorian::days(1), roundedNow));
  EditAccessPtr eda2 = boost::make_shared<EditAccess>(eda);
  WeatherDialog wd(eda2, s, limits, this);
  if (wd.exec()) {
    eda->newVersion();
    if (not eda2->sendChangesToParent(false)) {
      QMessageBox::critical(this,
          tr("WatchWeather"),
          tr("Sorry, your changes could not be saved and are lost!"),
          tr("OK"),
          "");
      }
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
    METLIBS_LOG_SCOPE();
    lity = lt;

    lstdlg->show();
}

void HqcMainWindow::startKro()
{
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
    METLIBS_LOG_SCOPE();
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
    METLIBS_LOG_WARN("error reading hqc_current_version, not renewing timer");
}

void HqcMainWindow::onShowErrorList()
{
  METLIBS_LOG_SCOPE();
  ui->dockErrors->setVisible(true);
}

void HqcMainWindow::onShowExtremeTAN()
{
  showExtremeValues(kvalobs::PARAMID_TAN);
}

void HqcMainWindow::onShowExtremeTAX()
{
  showExtremeValues(kvalobs::PARAMID_TAX);
}

void HqcMainWindow::onShowExtremeRR_24()
{
  showExtremeValues(kvalobs::PARAMID_RR_24);
}

void HqcMainWindow::showExtremeValues(int paramid)
{
  METLIBS_LOG_SCOPE();

  timeutil::ptime now = timeutil::now();
  int hour = now.time_of_day().hours();
  if (hour < 6)
    now -= boost::gregorian::days(1);
  
  const timeutil::ptime t1 = timeutil::from_YMDhms(now.date().year(), now.date().month(), now.date().day(), 6, 0, 0);
  const timeutil::ptime t0 = t1 - boost::gregorian::days(1);

  const TimeRange t(t0, t1);
  const std::vector<SensorTime> extremes = Extremes::find(paramid, t);
  mExtremesView->setExtremes(eda, extremes);
  mExtremesView->setVisible(true);
}

void HqcMainWindow::onShowChanges()
{
  METLIBS_LOG_SCOPE();
  ui->dockHistory->setVisible(true);
}

void HqcMainWindow::onShowSimpleCorrections()
{
  METLIBS_LOG_SCOPE();
  ui->dockCorrections->setVisible(true);
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
        METLIBS_LOG_WARN("Error in station lookup: " << e.what());
    }
}

void HqcMainWindow::tileHorizontal()
{
#if 1
  ui->ws->tileSubWindows();
#else
  QList<QMdiSubWindow *> allWindows = ui->ws->subWindowList();
  QList<QMdiSubWindow *> windows;
  BOOST_FOREACH(QMdiSubWindow* sw, allWindows) {
    if (sw->isVisible())
      windows << sw;
  }

  if (windows.empty())
    return;

  const int nWindows = windows.size();
  if (windows.size() == 1) {
    windows.front()->showMaximized();
    return;
  }

  const int FRAME_HEIGHT = windows.front()->parentWidget()->baseSize().height(), height = ui->ws->height() / nWindows;
  int y = 0;

  BOOST_FOREACH(QMdiSubWindow* sw, allWindows) {
    if (sw->windowState() == Qt::WindowMaximized)
      // prevent flicker
      sw->hide();

    sw->showNormal();
    const int h = std::max(height, sw->minimumHeight() + FRAME_HEIGHT);
    sw->setGeometry(0, y, ui->ws->width(), h);
    y += h;
  }
#endif
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
    if (eda->canUndo())
        eda->undoVersion();
}

void HqcMainWindow::onRedoChanges()
{
    if (eda->canRedo())
        eda->redoVersion();
}

void HqcMainWindow::onSaveChanges()
{
  if (not eda->sendChangesToParent()) {
    QMessageBox::critical(this, tr("HQC - Saving data"),
        tr("Sorry, your changes could not be saved!"),
        tr("OK"),
        "");
  } else {
    mHints->addHint(tr("<h1>Data Saved</h1>"
            "Your changes have been saved."));
  }
}

void HqcMainWindow::closeEvent(QCloseEvent* event)
{
  METLIBS_LOG_SCOPE();

  const int updates = eda->countU();
  if (updates > 0) {
    QMessageBox w(this);
    w.setWindowTitle(windowTitle());
    w.setIcon(QMessageBox::Warning);
    w.setText(tr("There are %1 unsaved data updates.").arg(updates));
    w.setInformativeText(tr("Are you sure that you want to lose them?"));
    QPushButton* discard = w.addButton(tr("Discard changes"), QMessageBox::ApplyRole);
    QPushButton* cont = w.addButton(tr("Continue"), QMessageBox::RejectRole);
    w.setDefaultButton(cont);
    w.exec();
    if (w.clickedButton() != discard) {
      event->ignore();
      return;
    }
  }
  writeSettings();
  event->accept();
}

void HqcMainWindow::navigateTo(const SensorTime& st)
{
    METLIBS_LOG_SCOPE();
    METLIBS_LOG_DEBUG(LOGVAL(st));

    BusyStatus busy(this, tr("Preparing data for station %1 at %2, please wait...")
        .arg(st.sensor.stationId)
        .arg(QString::fromStdString(timeutil::to_iso_extended_string(st.time))));
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
    METLIBS_LOG_DEBUG(LOGVAL(eda->countU()));
    ui->saveAction->setEnabled(eda->countU() > 0);
    ui->actionUndo->setEnabled(eda->canUndo());
    ui->actionRedo->setEnabled(eda->canRedo());

    ui->treeChanges->expandToDepth(2);
}


void HqcMainWindow::writeSettings()
{
    QSettings settings;
    settings.setValue(SETTING_HQC_GEOMETRY, saveGeometry());

    lstdlg->saveSettings(settings);
}

void HqcMainWindow::readSettings()
{
    METLIBS_LOG_SCOPE();

    QSettings settings;
    if (not restoreGeometry(settings.value(SETTING_HQC_GEOMETRY).toByteArray()))
        METLIBS_LOG_WARN("cannot restore hqc main window geometry");

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
