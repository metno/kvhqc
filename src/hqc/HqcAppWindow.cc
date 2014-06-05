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

#include "HqcAppWindow.hh"

#include "config.h"

#include "EditVersionModel.hh"
#include "EditVersionsView.hh"
#include "UserSettings.hh"
#include "SearchWindow.hh"

#include "common/HqcApplication.hh"

#include "util/ActionButton.hh"
#include "util/Helpers.hh"
#include "util/hqc_paths.hh"

#ifdef ENABLE_WATCHRR
#include "watchrr/StationDialog.hh"
#include "watchrr/WatchRRDialog.hh"
#endif // ENABLE_WATCHRR

#ifdef ENABLE_TEXTDATA
#include "textdata/TextdataDialog.hh"
#include "textdata/TextdataTable.hh"
#endif // ENABLE_TEXTDATA

#ifdef ENABLE_REJECTEDOBS
#include "rejectedobs/RejectedObs.hh"
#include "rejectedobs/RejectedObsDialog.hh"
#endif // ENABLE_REJECTEDOBS

#include <qUtilities/qtHelpDialog.h>

#include <QtCore/QFile>
#include <QtCore/QSettings>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QTimer>
#include <QtCore/QUrl>
#include <QtGui/QCloseEvent>
#include <QtGui/QDesktopServices>
#include <QtGui/QLabel>
#include <QtGui/QMessageBox>

#include "ui_appwindow.h"

#define MILOGGER_CATEGORY "kvhqc.HqcAppWindow"
#include "common/ObsLogging.hh"

namespace {

const int VERSION_CHECK_TIMEOUT = 60*60*1000; // milliseconds

const char SETTING_HQC_POSITION[] = "position";
const char SETTING_VERSION[] = "version";
const char SETTING_VERSION_FULL[] = "version_full";

enum { HELP_NEWS, HELP_INTRO };

} // anonymous namespace

HqcAppWindow::HqcAppWindow()
  : ui(new Ui_AppWindow)
  , mVersionCheckTimer(new QTimer(this))
  , mHelpDialog(0)
{
  METLIBS_LOG_SCOPE();
  setCaption(tr("HQC %1").arg(hqcApp->instanceName()));
  ui->setupUi(this);
  EditVersionsView* evv = new EditVersionsView(hqcApp->editAccess(), ui->groupChanges);
  ui->gridChanges->addWidget(evv, 1, 0, 1, 4);
  ui->gridChanges->setRowStretch(1, 1);

  mActionButtonSave = new ActionButton(ui->buttonSave, ui->actionSave, this);
  mActionButtonUndo = new ActionButton(ui->buttonUndo, ui->actionUndo, this);
  mActionButtonRedo = new ActionButton(ui->buttonRedo, ui->actionRedo, this);

  connect(mVersionCheckTimer, SIGNAL(timeout()), this, SLOT(onVersionCheckTimeout()));
  mVersionCheckTimer->setSingleShot(true);

  mKvalobsAvailable = new QLabel(statusBar());
  connect(hqcApp, SIGNAL(kvalobsAvailable(bool)), this, SLOT(kvalobsAvailable(bool)));
  statusBar()->addPermanentWidget(mKvalobsAvailable, 0);
  kvalobsAvailable(hqcApp->isKvalobsAvailable());

#ifdef ENABLE_TEXTDATA
  connect(ui->buttonTextData, SIGNAL(clicked()), this, SLOT(onNewTextdata()));
#else
  ui->buttonTextData->setEnabled(false);
#endif // ENABLE_TEXTDATA

#ifdef ENABLE_REJECTEDOBS
  connect(ui->buttonRejected, SIGNAL(clicked()), this, SLOT(onNewRejectedObs()));
#else
  ui->buttonRejected->setEnabled(false);
#endif // ENABLE_REJECTEDOBS

  connect(hqcApp->editAccess().get(), SIGNAL(currentVersionChanged(size_t, size_t)),
      this, SLOT(onEditVersionChanged(size_t, size_t)));
  // initialize save/undo/redo buttons
  onEditVersionChanged(0, 0);
}

HqcAppWindow::~HqcAppWindow()
{
}

void HqcAppWindow::changeEvent(QEvent *event)
{
  if (event->type() == QEvent::LanguageChange)
    retranslateUi();
  QMainWindow::changeEvent(event);
}

void HqcAppWindow::retranslateUi()
{
  ui->retranslateUi(this);
  kvalobsAvailable(hqcApp->isKvalobsAvailable());
}

void HqcAppWindow::startup()
{
  METLIBS_LOG_SCOPE();
    
  readSettings();
  show();
  checkVersionSettings();

  statusBar()->message(tr("Welcome to kvhqc %1!").arg(PVERSION_FULL), 2000);
  mVersionCheckTimer->start(VERSION_CHECK_TIMEOUT);
}

void HqcAppWindow::finish()
{
  METLIBS_LOG_SCOPE();
  writeSettings();
}

void HqcAppWindow::onNewSearch()
{
  SearchWindow* sw = new SearchWindow(this);
  sw->show();
}

void HqcAppWindow::onNewTextdata()
{
#ifdef ENABLE_TEXTDATA
  TextData::showTextData(txtdlg->getStationId(), txtdlg->getTimeSpan(), this);
#endif
}

void HqcAppWindow::onNewRejectedObs()
{
#ifdef ENABLE_REJECTEDOBS
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

void HqcAppWindow::onNewWatchRR()
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

  WatchRRDialog watchrr(eda, kma, sensor, time, this);
  watchrr.show();
#endif // ENABLE_WATCHRR
}

void HqcAppWindow::onStartKro()
{
  QDesktopServices::openUrl(QUrl("http://kro/cgi-bin/start.pl"));
}

void HqcAppWindow::onVersionCheckTimeout()
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

void HqcAppWindow::onHelpUse()
{
#if 0
  QDesktopServices::openUrl(QUrl("https://dokit.met.no/klima/tools/qc/hqc-help"));
#else
  showHelpDialog(HELP_INTRO);
#endif
}

void HqcAppWindow::onHelpFlag()
{
  QDesktopServices::openUrl(QUrl("https://kvalobs.wiki.met.no/doku.php?id=kvalobs:kvalobs-flagg"));
}

void HqcAppWindow::onHelpParam()
{
  QDesktopServices::openUrl(QUrl("https://kvalobs.wiki.met.no/doku.php?id=kvalobs:kvalobs-parametre_sortert_alfabetisk_etter_kode"));
}

void HqcAppWindow::onAboutHqc()
{
  QMessageBox::about( this, tr("About Hqc"),
      tr("Hqc is a program for manual quality control of observations. "
          "The program consists of editable tables with observations including "
          "a time series diagram, and it can be connected to Diana."
          "\n\n"
          "The program is developed by "
          "Knut Johansen, "
          "Alexander B�rger, "
          "Lisbeth Bergholt, "
          "Vegard B�nes, "
          "Audun Christoffersen at MET Norway.\n\n"
          "You are using HQC version %1.").arg(PVERSION_FULL));
}

void HqcAppWindow::onAboutQt()
{
  QMessageBox::aboutQt(this);
}

void HqcAppWindow::onUndoChanges()
{
  EditAccess_p eda = hqcApp->editAccess();
  if (eda->canUndo())
    eda->undoVersion();
}

void HqcAppWindow::onRedoChanges()
{
  EditAccess_p eda = hqcApp->editAccess();
  if (eda->canRedo())
    eda->redoVersion();
}

void HqcAppWindow::onSaveChanges()
{
  EditAccess_p eda = hqcApp->editAccess();
  if (not eda->storeToBackend()) {
    QMessageBox::critical(this, tr("HQC - Saving data"),
        tr("Sorry, your changes could not be saved!"),
        tr("OK"),
        "");
  }
}

void HqcAppWindow::kvalobsAvailable(bool available)
{
  if (available) {
    mKvalobsAvailable->setPixmap(QPixmap("icons:kv_ok.svg"));
    mKvalobsAvailable->setToolTip(tr("Kvalobs seems to be available"));
  } else {
    mKvalobsAvailable->setPixmap(QPixmap("icons:kv_error.svg"));
    mKvalobsAvailable->setToolTip(tr("Kvalobs seems not to be available"));
  }
}

void HqcAppWindow::closeEvent(QCloseEvent* event)
{
  METLIBS_LOG_SCOPE();

  EditAccess_p eda = hqcApp->editAccess();
  if (Helpers::askDiscardChanges(eda->countU(), this)) {
    writeSettings();
    event->accept();
  } else {
    event->ignore();
  }
}

void HqcAppWindow::onEditVersionChanged(size_t current, size_t highest)
{
  METLIBS_LOG_SCOPE();
  EditAccess_p eda = hqcApp->editAccess();
  METLIBS_LOG_DEBUG(LOGVAL(eda->countU()));
  ui->actionSave->setEnabled(eda->canUndo());
  ui->actionUndo->setEnabled(eda->canUndo());
  ui->actionRedo->setEnabled(eda->canRedo());
}

void HqcAppWindow::writeSettings()
{
  QSettings settings;
  settings.setValue(SETTING_HQC_POSITION, pos());
}

void HqcAppWindow::readSettings()
{
  METLIBS_LOG_SCOPE();

  QSettings settings;
  move(settings.value(SETTING_HQC_POSITION).toPoint());
}

void HqcAppWindow::onUserSettings()
{
  UserSettings us;
  us.exec();
}

void HqcAppWindow::checkVersionSettings()
{
  QSettings settings;

  QString savedVersionFull = settings.value(SETTING_VERSION_FULL, "??").toString();
  if (savedVersionFull != PVERSION_FULL) {
    settings.setValue(SETTING_VERSION_FULL, PVERSION_FULL);
    onHelpNews();
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

void HqcAppWindow::onHelpNews()
{
  showHelpDialog(HELP_NEWS, PVERSION_FULL);
}

void HqcAppWindow::showHelpDialog(int doc, const std::string& anchor)
{
  if (not mHelpDialog) {
    HelpDialog::Info info;
    info.path = (::hqc::getPath(::hqc::DOCDIR) + "/html").toStdString();

    HelpDialog::Info::Source helpsource;
    helpsource.source = "news.html";
    helpsource.name = "Kvhqc News";
    helpsource.defaultlink = "";
    info.src.push_back(helpsource);

    helpsource.source = "intro.html";
    helpsource.name = "Kvhqc Introduction";
    helpsource.defaultlink = "";
    info.src.push_back(helpsource);

    mHelpDialog = new HelpDialog(this, info);
  }
  mHelpDialog->showdoc(doc, anchor);
}