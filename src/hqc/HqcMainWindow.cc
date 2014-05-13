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

#include "HqcMainWindow.hh"

#include "config.h"

#include "AutoDataList.hh"
#include "EditVersionModel.hh"
#include "EditVersionsView.hh"
#include "JumpToObservation.hh"
#include "ListDialog.hh"
#include "StaticDataList.hh"
#include "TimeSeriesView.hh"
#include "UserSettings.hh"

#include "common/CachingAccess.hh"
#include "common/KvalobsAccess.hh"

#include "common/KvalobsModelAccess.hh"
#include "common/KvHelpers.hh"
#include "common/KvMetaDataBuffer.hh"
#include "common/KvServiceHelper.hh"
#include "common/HqcApplication.hh"

#include "util/Helpers.hh"
#include "util/hqc_paths.hh"
#include "util/timeutil.hh"
#include "util/BusyIndicator.hh"
#include "util/EtaProgressDialog.hh"
#include "util/HintWidget.hh"
#include "util/QNoCloseMdiSubWindow.hh"
#include "util/UiHelpers.hh"

#ifdef ENABLE_WATCHRR
#include "watchrr/StationDialog.hh"
#include "watchrr/WatchRRDialog.hh"
#endif // ENABLE_WATCHRR

#ifdef ENABLE_WATCHWEATHER
#include "weather/WeatherDialog.hh"
#include "weather/WeatherStationDialog.hh"
#endif // ENABLE_WATCHWEATHER

#ifdef ENABLE_ERRORLIST
#include "errorlist/ErrorList.hh"
#endif // ENABLE_ERRORLIST

#ifdef ENABLE_DIANA
#include "HqcDianaHelper.hh"
#include <coserver/ClientButton.h>
#endif // ENABLE_DIANA

#ifdef ENABLE_EXTREMES
#include "extremes/ExtremesView.hh"
#endif // ENABLE_EXTREMES

#ifdef ENABLE_MISSINGOBS
#include "missing/MissingView.hh"
#endif // ENABLE_MISSINGOBS

#ifdef ENABLE_TEXTDATA
#include "TextdataDialog.hh"
#include "TextdataTable.hh"
#endif // ENABLE_TEXTDATA

#ifdef ENABLE_REJECTEDOBS
#include "RejectedObs.hh"
#include "RejectedObsDialog.hh"
#endif // ENABLE_REJECTEDOBS

#ifdef ENABLE_SIMPLECORRECTIONS
#include "SimpleCorrections.hh"
#endif // ENABLE_SIMPLECORRECTIONS

#include <qUtilities/qtHelpDialog.h>

#include <QtCore/qfile.h>
#include <QtCore/qsettings.h>
#include <QtCore/QTextStream>
#include <QtCore/qtimer.h>
#include <QtCore/qurl.h>
#include <QtGui/QDesktopServices>
#include <QtGui/QDockWidget>
#include <QtGui/QLabel>
#include <QtGui/QMdiSubWindow>
#include <QtGui/QMessageBox>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtGui/QPrinter>
#include <QtGui/QPrintDialog>
#include <QtGui/QProgressDialog>
#include <QtGui/QSplitter>

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

#include "ui_mainwindow.h"

#define MILOGGER_CATEGORY "kvhqc.HqcMainWindow"
#include "common/ObsLogging.hh"

namespace {

const int VERSION_CHECK_TIMEOUT = 60*60*1000; // milliseconds

const char SETTING_HQC_GEOMETRY[] = "geometry";
const char SETTING_HQC_AUTOVIEW_SPLITTER[] = "autoview_slider";
const char SETTING_VERSION[] = "version";
const char SETTING_VERSION_FULL[] = "version_full";

} // anonymous namespace

HqcMainWindow::HqcMainWindow()
  : QMainWindow( 0, tr("HQC"))
  , listExist(false)
  , ui(new Ui::HqcMainWindow)
  , mVersionCheckTimer(new QTimer(this))
  , mHints(new HintWidget(this))
  , kda(boost::make_shared<KvalobsAccess>())
  , cda(boost::make_shared<CachingAccess>(kda))
  , kma(boost::make_shared<KvalobsModelAccess>())
  , eda(boost::make_shared<EditAccess>(cda))
  , mEditVersions(new EditVersionModel(eda, this))
  , mProgressDialog(new EtaProgressDialog(this))
  , mAutoDataList(new AutoDataList(this))
  , mTimeSeriesView(new TimeSeriesView(this))
  , mDockSearch(0)
{
  METLIBS_LOG_SCOPE();
  ui->setupUi(this);

#ifdef ENABLE_ERRORLIST
  { mErrorsView = new ErrorList(this);
    mErrorsView->setDataAccess(eda, kma);
    connect(mErrorsView, SIGNAL(signalNavigateTo(const SensorTime&)), this, SLOT(navigateTo(const SensorTime&)));
    addSearchDock(mErrorsView, true);
  }
#endif // ENABLE_ERRORLIST

#ifdef ENABLE_EXTREMES
  { mExtremesView = new ExtremesView(this);
    mExtremesView->setDataAccess(eda);
    mExtremesView->signalNavigateTo.connect(boost::bind(&HqcMainWindow::navigateTo, this, _1));
    addSearchDock(mExtremesView);
  }
#endif // ENABLE_EXTREMES

#ifdef ENABLE_MISSINGOBS
  { mMissingView = new MissingView(this);
    mMissingView->setDataAccess(eda);
    mMissingView->signalNavigateTo.connect(boost::bind(&HqcMainWindow::navigateTo, this, _1));
    addSearchDock(mMissingView);
  }
#endif // ENABLE_MISSINGOBS

  ui->menuView->addSeparator();

  { EditVersionsView* evv = new EditVersionsView(mEditVersions, this);
    addDock(evv, Qt::RightDockWidgetArea);
    connect(evv, SIGNAL(saveRequested()), this, SLOT(onSaveChanges()));
  }

#ifdef ENABLE_SIMPLECORRECTIONS
  { mCorrections = new SimpleCorrections(this);
    mCorrections->setDataAccess(eda, kma);
    addDock(mCorrections, Qt::BottomDockWidgetArea, true);
  }
#endif

  ui->actionRedo->setIcon(QIcon("icons:redo.svg"));
  ui->actionUndo->setIcon(QIcon("icons:undo.svg"));

  connect(mVersionCheckTimer, SIGNAL(timeout()), this, SLOT(onVersionCheckTimeout()));
  mVersionCheckTimer->setSingleShot(true);

#ifdef ENABLE_DIANA
  pluginB = new ClientButton("hqc", "/usr/bin/coserver4", statusBar());
  statusBar()->addPermanentWidget(pluginB, 0);
  mDianaHelper.reset(new HqcDianaHelper(pluginB));
  mDianaHelper->setDataAccess(eda, kma);
  mDianaHelper->signalNavigateTo.connect(boost::bind(&HqcMainWindow::navigateTo, this, _1));
#endif

  mKvalobsAvailable = new QLabel(statusBar());
  connect(hqcApp, SIGNAL(kvalobsAvailable(bool)), this, SLOT(kvalobsAvailable(bool)));
  statusBar()->addPermanentWidget(mKvalobsAvailable, 0);
  kvalobsAvailable(hqcApp->isKvalobsAvailable());

  lstdlg = new ListDialog(this);
  lstdlg->hide();
  connect(lstdlg, SIGNAL(accepted()), this, SLOT(ListOK()));

#ifdef ENABLE_TEXTDATA
  txtdlg = new TextDataDialog(this);
  txtdlg->hide();
  connect(ui->actionTextDataList, SIGNAL(triggered()), txtdlg, SLOT(show()));
  connect(txtdlg, SIGNAL(textDataApply()), SLOT(textDataOK()));
#endif // ENABLE_TEXTDATA

#ifdef ENABLE_REJECTEDOBS
  rejdlg = new RejectedObsDialog(this);
  rejdlg->hide();
  connect(ui->actionRejectDecode, SIGNAL(triggered()), rejdlg, SLOT(show()));
  connect(rejdlg, SIGNAL(rejectApply()), SLOT(rejectedOK()));
#endif // ENABLE_REJECTEDOBS

  mAutoDataList->setDataAccess(eda, kma);
  connect(mAutoDataList, SIGNAL(signalNavigateTo(const SensorTime&)),
      this, SLOT(navigateTo(const SensorTime&)));

  mTimeSeriesView->setDataAccess(eda, kma);

  mJumpToObservation = new JumpToObservation(cda, this);
  connect(mJumpToObservation, SIGNAL(signalNavigateTo(const SensorTime&)),
      this, SLOT(navigateTo(const SensorTime&)));

  mAutoViewSplitter = new QSplitter(ui->tabs);
  mAutoViewSplitter->addWidget(mAutoDataList);
  mAutoViewSplitter->addWidget(mTimeSeriesView);
  mAutoViewSplitter->setOpaqueResize(false);
  ui->tabs->addTab(mAutoViewSplitter, tr("Auto List/Series"));

  connect(eda.get(), SIGNAL(currentVersionChanged(size_t, size_t)),
      this, SLOT(onEditVersionChanged(size_t, size_t)));
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

  mProgressDialog->setWindowModality(Qt::ApplicationModal);
  mProgressDialog->setWindowTitle(tr("HQC"));
  mProgressDialog->setLabelText(tr("Fetching data, please wait ..."));
  mProgressDialog->setCancelButton(0); // cancel is not possible yet
  mProgressDialog->setMinimumDuration(4000);
  //kda->signalFetchingData.connect(boost::bind(&HqcMainWindow::onKvalobsFetchingData, this, _1, _2));
}

HqcMainWindow::~HqcMainWindow()
{
  //kda->signalFetchingData.disconnect(boost::bind(&HqcMainWindow::onKvalobsFetchingData, this, _1, _2));

  //mJumpToObservation->signalNavigateTo.disconnect(boost::bind(&HqcMainWindow::navigateTo, this, _1));
  //mAutoDataList     ->signalNavigateTo.disconnect(boost::bind(&HqcMainWindow::navigateTo, this, _1));
#ifdef ENABLE_MISSINGOBS
  mMissingView      ->signalNavigateTo.disconnect(boost::bind(&HqcMainWindow::navigateTo, this, _1));
#endif
#ifdef ENABLE_EXTREMES
  mExtremesView     ->signalNavigateTo.disconnect(boost::bind(&HqcMainWindow::navigateTo, this, _1));
#endif
#ifdef ENABLE_DIANA
  mDianaHelper      ->signalNavigateTo.disconnect(boost::bind(&HqcMainWindow::navigateTo, this, _1));
#endif
}

QDockWidget* HqcMainWindow::addSearchDock(QWidget* view, bool visible)
{
  QDockWidget* dock = addDock(view, Qt::BottomDockWidgetArea, visible);
  if (mDockSearch)
    tabifyDockWidget(mDockSearch, dock);
  else
    mDockSearch = dock;
  return dock;
}

QDockWidget* HqcMainWindow::addDock(QWidget* view, Qt::DockWidgetArea area, bool visible)
{
  QDockWidget* dock = new QDockWidget(this);
  dock->setVisible(visible);
  dock->setWidget(view);
  dock->setWindowTitle(view->windowTitle());
  addDockWidget(area, dock);
  ui->menuView->addAction(dock->toggleViewAction());
  mDocks.append(dock);
  return dock;
}

void HqcMainWindow::changeEvent(QEvent *event)
{
  if (event->type() == QEvent::LanguageChange) {
    ui->retranslateUi(this);

    ui->tabs->setTabText(0, tr("Auto List/Series"));
    for (int i=1; i<ui->tabs->count(); ++i)
      ui->tabs->setTabText(i, tr("Selected Data"));

    mProgressDialog->setWindowTitle(tr("HQC"));
    mProgressDialog->setLabelText(tr("Fetching data, please wait ..."));

    for (int i=0; i<mDocks.size(); ++i) {
      QDockWidget* dock = mDocks[i];
      dock->setWindowTitle(dock->widget()->windowTitle());
    }

    kvalobsAvailable(hqcApp->isKvalobsAvailable());
  }
  QMainWindow::changeEvent(event);
}

void HqcMainWindow::setReinserter(AbstractReinserter_p r)
{
  kda->setReinserter(r);
}

void HqcMainWindow::startup(const QString& captionSuffix)
{
  METLIBS_LOG_SCOPE();
  setCaption("HQC " + captionSuffix);
    
  DisableGUI disableGUI(this);
  listExist = false;

  //-----------------------------------------------------------------

  readSettings();
  show();
  checkVersionSettings();
  Helpers::processNonUserEvents();

  statusBar()->message( tr("Welcome to kvhqc %1!").arg(PVERSION_FULL), 2000 );
  mVersionCheckTimer->start(VERSION_CHECK_TIMEOUT);
}

void HqcMainWindow::ListOK()
{
  METLIBS_LOG_SCOPE();
#ifdef ENABLE_DIANA
  if (not mDianaHelper->isConnected()) {
    mHints->addHint(tr("<h1>Diana-Connection</h1>"
            "No contact with diana! "
            "You should connect to the command server via the button in the lower right in the hqc window, "
            "and connect diana to the command server using the button in diana's window."));
  }
#endif // ENABLE_DIANA
  // FIXME pack selectedStations, selectedTimes, ... in class, pass this to AnalyseErrors
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

  const TimeSpan timeLimits = lstdlg->getTimeSpan();

  Sensor_v sensors;
  {
    BusyStatus busySensors(this, tr("Building station list..."));
    BOOST_FOREACH(int stationId, selectedStations) {
      BOOST_FOREACH(int paramId, mSelectedParameters) {
        const KvMetaDataBuffer::ObsPgmList& opl = KvMetaDataBuffer::instance()->findObsPgm(stationId);
        Sensor sensor(stationId, paramId, 0, 0, 0);
        std::set<int> typeIdsShown;
        BOOST_FOREACH(const kvalobs::kvObsPgm& op, opl) {
          const TimeSpan op_time(op.fromtime(), op.totime());
          if (timeLimits.intersection(op_time).undef())
            continue;
          const int p = op.paramID(), t = op.typeID();
          if (p == paramId) {
            sensor.typeId = t;
          } else if (Helpers::aggregatedParameter(p, paramId)) {
            sensor.typeId = -t;
          } else {
            continue;
          }
          if (typeIdsShown.find(sensor.typeId) == typeIdsShown.end()) {
            sensors.push_back(sensor);
            typeIdsShown.insert(sensor.typeId);
          }
        }
      }
    }
  }

#ifdef ENABLE_ERRORLIST
  if (lity == erLi or lity == erSa or lity == alLi or lity == alSa) {
    BusyStatus busyErrors(this, tr("Building error list..."));

    mErrorsView->setErrorsForSalen(lity == erSa or lity == alSa);
    mErrorsView->setSensorsAndTimes(sensors, timeLimits);
  }
#endif // ENABLE_ERRORLIST

  if (lity == daLi or lity == alLi or lity == alSa) {
    BusyStatus busyData(this, tr("Building data list..."));

    StaticDataList* dl = new StaticDataList(this);
    dl->setDataAccess(eda, kma);
    dl->setSensorsAndTimes(sensors, timeLimits);
    //dl->signalNavigateTo.connect(boost::bind(&HqcMainWindow::navigateTo, this, _1));
    ui->tabs->addTab(dl, tr("Selected Data"));
  }

  std::vector<QString> stationList;
  BOOST_FOREACH(int stnr, selectedStations) {
    try {
      const kvalobs::kvStation& station = KvMetaDataBuffer::instance()->findStation(stnr);
      const QString statId = QString::number(stnr) + " " + QString::fromStdString(station.name());
      stationList.push_back(statId);
    } catch (std::exception& e) {
      HQC_LOG_WARN("Error in lookup for station " << stnr << ", exception is: " << e.what());
    }
  }
  Q_EMIT newStationList(stationList);
  METLIBS_LOG_DEBUG("newStationList emitted");

  //  send parameter names to ts dialog
  Q_EMIT newParameterList(mSelectedParameters);
#ifdef ENABLE_DIANA
  if (lity != erLi && lity != erSa)
    mDianaHelper->setSensorsAndTimes(sensors, timeLimits);
#endif

  statusBar()->message("");
}

void HqcMainWindow::textDataOK()
{
#ifdef ENABLE_TEXTDATA
  TextData::showTextData(txtdlg->getStationId(), txtdlg->getTimeSpan(), this);
#endif
}

void HqcMainWindow::rejectedOK()
{
#ifdef ENABLE_REJECTEDOBS
  METLIBS_LOG_SCOPE();

  try {
    std::list<kvalobs::kvRejectdecode> rejectList;
    const TimeSpan t = rejdlg->getTimeSpan();
    if (KvServiceHelper::instance()->getKvRejectDecode(rejectList, t)) {
      std::string decoder = "comobs";
      std::vector<kvalobs::kvRejectdecode> rejList;
      BOOST_FOREACH(const kvalobs::kvRejectdecode& reject, rejectList) {
        if (reject.decoder().substr(0, decoder.size()) != decoder)
          continue;
        if (reject.comment() == "No decoder for SMS code <12>!")
          continue;
          
        METLIBS_LOG_INFO(reject.tbtime() << ' ' << reject.message() << ' ' << reject.comment() << reject.decoder());
        rejList.push_back(reject);
      }
      new Rejects(rejList, this);
      return;
    }
  } catch (std::exception& e) {
    HQC_LOG_ERROR("exception while retrieving rejectdecode data: " << e.what());
  }
  // reach here in case of error
  QMessageBox::critical(this, tr("No RejectDecode"), tr("Could not read rejectdecode."),
      QMessageBox::Ok, QMessageBox::NoButton);
#endif // ENABLE_REJECTEDOBS
}

void HqcMainWindow::showWatchRR()
{
#ifdef ENABLE_WATCHRR
  const timeutil::ptime now = timeutil::now();

  Sensor sensor(83880, kvalobs::PARAMID_RR_24, 0, 0, 302);
  timeutil::ptime tMiddle = now - boost::gregorian::days(7);
  if (mLastNavigated.valid()) {
    sensor = mLastNavigated.sensor;
    sensor.paramId = kvalobs::PARAMID_RR_24;
    tMiddle = timeutil::from_miTime(mLastNavigated.time);
  }

  timeutil::ptime timeTo = timeutil::ptime(tMiddle.date(), boost::posix_time::hours(6)) + boost::gregorian::days(7);
  timeutil::ptime timeFrom = timeTo - boost::gregorian::days(21);
  while (timeTo > now)
    timeTo -= boost::gregorian::days(1);
  TimeSpan time(timeFrom, timeTo);

  StationDialog sd(sensor, time);
  if (not sd.exec())
    return;
  sensor = sd.selectedSensor();
  time = sd.selectedTime();

  mDianaHelper->setEnabled(false);
  WatchRRDialog watchrr(eda, kma, sensor, time, this);
  watchrr.exec();
  mDianaHelper->setEnabled(true);
#endif // ENABLE_WATCHRR
}

void HqcMainWindow::showWeather()
{
#ifdef ENABLE_WATCHWEATHER
  const timeutil::ptime now = timeutil::now();
  const timeutil::ptime roundedNow = timeutil::ptime(now.date(), boost::posix_time::time_duration(now.time_of_day().hours(),0,0))
      + boost::posix_time::hours(1);

  Sensor sensor(10380, kvalobs::PARAMID_TA,    0, 0, 311);
  timeutil::ptime tMiddle = roundedNow - boost::gregorian::days(7);
  if (mLastNavigated.valid()) {
    sensor = mLastNavigated.sensor;
    tMiddle = timeutil::from_miTime(mLastNavigated.time);
  }

  timeutil::ptime timeTo = timeutil::ptime(tMiddle.date(), boost::posix_time::hours(6)) + boost::gregorian::days(7);
  timeutil::ptime timeFrom = timeTo - boost::gregorian::days(21);
  while (timeTo > now)
    timeTo -= boost::gregorian::days(1);
  TimeSpan time(timeFrom, timeTo);
  
  WeatherStationDialog sd(sensor, time);
  if (not sd.exec())
    return;
  sensor = sd.selectedSensor();
  time = sd.selectedTime();

  WeatherDialog* wd = new WeatherDialog(eda, sensor, time, this);
  wd->show();
#endif // ENABLE_WATCHWEATHER
}

void HqcMainWindow::errListMenu()
{
  listMenu(erLi);
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
  HQC_LOG_WARN("error reading hqc_current_version, not renewing timer");
}

void HqcMainWindow::onKvalobsFetchingData(int total, int ready)
{
  METLIBS_LOG_SCOPE();
  METLIBS_LOG_DEBUG(LOGVAL(total) << LOGVAL(ready));
  if (total > 0) {
    if (mProgressDialog->maximum() != total)
      mProgressDialog->setMaximum(total);
    mProgressDialog->setValue(ready);
  } else {
    mProgressDialog->reset();
  }
}

void HqcMainWindow::onJumpToObservation()
{
  METLIBS_LOG_SCOPE();
  mJumpToObservation->exec();
}

void HqcMainWindow::onTabCloseRequested(int index)
{
  QWidget* w = ui->tabs->widget(index);
  if (w != mAutoViewSplitter) {
    ui->tabs->removeTab(index);
    BusyIndicator busy;
    delete w;
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
          "Audun Christoffersen at MET Norway.\n\n"
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
  if (not eda->storeToBackend()) {
    QMessageBox::critical(this, tr("HQC - Saving data"),
        tr("Sorry, your changes could not be saved!"),
        tr("OK"),
        "");
  } else {
    mHints->addHint(tr("<h1>Data Saved</h1>"
            "Your changes have been saved."));
  }
}

void HqcMainWindow::kvalobsAvailable(bool available)
{
  if (available) {
    mKvalobsAvailable->setPixmap(QPixmap("icons:kv_ok.svg"));
    mKvalobsAvailable->setToolTip(tr("Kvalobs seems to be available"));
  } else {
    mKvalobsAvailable->setPixmap(QPixmap("icons:kv_error.svg"));
    mKvalobsAvailable->setToolTip(tr("Kvalobs seems not to be available"));
    mHints->addHint(tr("<h1>Kvalobs unavailable</h1>"
            "Database not accessible right now! "
            "You may wait and try again, but you must expect any changes to be lost."));
  }
}

void HqcMainWindow::closeEvent(QCloseEvent* event)
{
  METLIBS_LOG_SCOPE();

  if (Helpers::askDiscardChanges(eda->countU(), this)) {
    writeSettings();
    event->accept();
  } else {
    event->ignore();
  }
}

void HqcMainWindow::navigateTo(const SensorTime& st)
{
  METLIBS_LOG_TIME();
  METLIBS_LOG_DEBUG(LOGVAL(st));

  if (eq_SensorTime()(mLastNavigated, st))
    return;

#if 1
  BusyIndicator busy;
#else
  // this disables the GUI and is therefore often much slower than updating the views
  BusyStatus busy(this, tr("Preparing data for station %1 at %2, please wait...")
      .arg(st.sensor.stationId)
      .arg(QString::fromStdString(timeutil::to_iso_extended_string(st.time))));
#endif
  mLastNavigated = st;

#ifdef ENABLE_DIANA
  mDianaHelper->navigateTo(st);
#endif
#ifdef ENABLE_ERRORLIST
  mErrorsView->navigateTo(st);
#endif
#ifdef ENABLE_SIMPLECORRECTIONS
  mCorrections->navigateTo(st);
#endif
  mJumpToObservation->navigateTo(st);
  mAutoDataList->navigateTo(st);
  mTimeSeriesView->navigateTo(st);
  for(int idx = 1; idx < ui->tabs->count(); ++idx) {
    QWidget* w = ui->tabs->widget(idx);
    if (StaticDataList* sdl = static_cast<StaticDataList*>(w)) {
      sdl->navigateTo(st);
    }
  }
}

void HqcMainWindow::onEditVersionChanged(size_t current, size_t highest)
{
  METLIBS_LOG_DEBUG(LOGVAL(eda->countU()));
  ui->saveAction->setEnabled(eda->canUndo());
  ui->actionUndo->setEnabled(eda->canUndo());
  ui->actionRedo->setEnabled(eda->canRedo());
}

void HqcMainWindow::writeSettings()
{
  QSettings settings;
  settings.setValue(SETTING_HQC_GEOMETRY, saveGeometry());
  settings.setValue(SETTING_HQC_AUTOVIEW_SPLITTER, mAutoViewSplitter->saveState());

  lstdlg->saveSettings(settings);
}

void HqcMainWindow::readSettings()
{
  METLIBS_LOG_SCOPE();

  QSettings settings;
  if (not restoreGeometry(settings.value(SETTING_HQC_GEOMETRY).toByteArray()))
    HQC_LOG_WARN("cannot restore hqc main window geometry");
  if (not mAutoViewSplitter->restoreState(settings.value(SETTING_HQC_AUTOVIEW_SPLITTER).toByteArray()))
    HQC_LOG_WARN("cannot restore autoview splitter positions");

  lstdlg->restoreSettings(settings);
}

void HqcMainWindow::onUserSettings()
{
  METLIBS_LOG_SCOPE();
  UserSettings us;
  us.exec();
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
