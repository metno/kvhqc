/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2018 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: kvalobs-dev@met.no

  This file is part of HQC

  HQC is free software; you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  HQC is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General Public License
  along with HQC; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include "HqcApplication.hh"

#include "CachingAccess.hh"
#include "EditAccess.hh"
#include "HqcUserConfig.hh"
#include "KvServiceHelper.hh"
#include "KvalobsAccess.hh"
#include "KvalobsModelAccess.hh"
#include "KvalobsQueryRunner.hh"
#include "TimeSpan.hh"
#include "util/Helpers.hh"
#include "util/hqc_paths.hh"
#include "util/stringutil.hh"

#include <kvalobs/kvStationParam.h>

#include <kvcpp/corba/CorbaKvApp.h>
#include <kvcpp/KvApp.h>

#include <puTools/miString.h>

#include <QDir>
#include <QIcon>
#include <QLibraryInfo>
#include <QLocale>
#include <QMessageBox>
#include <QSettings>
#include <QSqlError>
#include <QSqlQuery>
#include <QThread>
#include <QTimer>
#include <QTranslator>
#include <QUrl>
#include <QVariant>

#define MILOGGER_CATEGORY "kvhqc.HqcApplication"
#define M_TIME
#include "util/HqcLogging.hh"

namespace /* anonymous */ {
const char DB_SYSTEM[] = "hqc_system_db";
const char DB_CONFIG[] = "hqc_config_db";
const char DB_KVALOBS[] = "kvalobs_db";

const char CK_KVALOBSDB[] = "kvalobsdb";

const int AVAILABILITY_TIMEROUT = 120*1000; // milliseconds = 2 minutes

const char SETTING_HQC_LANGUAGE[] = "language";

} // anonymous namespace

HqcApplication* hqcApp = 0;

HqcApplication::HqcApplication(int& argc, char** argv, std::shared_ptr<miutil::conf::ConfSection> conf)
    : QApplication(argc, argv)
    , mConfig(conf)
{
  hqcApp = this;

  QCoreApplication::setOrganizationName("Meteorologisk Institutt");
  QCoreApplication::setOrganizationDomain("met.no");
  QCoreApplication::setApplicationName("Hqc");

  // must init after configuring QCoreApplication as it reads QSettings
  mUserConfig.reset(new HqcUserConfig);

  QString language = savedLanguage();
  if (not language.isEmpty())
    QLocale::setDefault(QLocale(language));
  installTranslations();

  QDir::setSearchPaths("icons", QStringList(hqc::getPath(hqc::IMAGEDIR)));
  setWindowIcon(QIcon("icons:hqc_logo.svg"));

  connect(this, SIGNAL(lastWindowClosed()), this, SLOT(quit()));

  QTimer* availabilityTimer = new QTimer(this);
  connect(availabilityTimer, SIGNAL(timeout()), this, SLOT(checkKvalobsAvailability()));
  availabilityTimer->start(AVAILABILITY_TIMEROUT);

  KvalobsQueryRunner_p kvalobsRunner = std::make_shared<KvalobsQueryRunner>();
  mKvalobsHandler = std::make_shared<QueryTaskHandler>(kvalobsRunner, true);
  kda = std::make_shared<KvalobsAccess>(mKvalobsHandler);
  cda = std::make_shared<CachingAccess>(kda);
  kma = std::make_shared<KvalobsModelAccess>(mKvalobsHandler);
  eda = std::make_shared<EditAccess>(cda);

  QObject::connect(KvServiceHelper::instance(), SIGNAL(kvalobsAvailable(bool)),
      this, SLOT(changedKvalobsAvailability(bool)));
}

HqcApplication::~HqcApplication()
{
  QSqlDatabase::removeDatabase(DB_SYSTEM);
  QSqlDatabase::removeDatabase(DB_CONFIG);

  hqcApp = 0;
}

QSqlDatabase HqcApplication::systemDB()
{
  if (not QSqlDatabase::contains(DB_SYSTEM)) {
    const QString dbPath = ::hqc::getPath(::hqc::DATADIR) + "/hqc_system.db";
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", DB_SYSTEM);
    db.setDatabaseName(dbPath);
    if ((not db.open()) or db.tables().empty()) {
      fatalError(tr("Cannot access hqc system database, please check the HQC installation"), dbPath);
    }
  }
  return QSqlDatabase::database(DB_SYSTEM);
}

QSqlDatabase HqcApplication::configDB()
{
  if (not QSqlDatabase::contains(DB_CONFIG)) {
    const std::string home = miutil::from_c_str(getenv("HOME"));
    if (home.empty()) {
      fatalError(tr("No $HOME enviroment, please check your computer's setup"));
      // not reached
    }

    const QString dbPath = Helpers::fromUtf8(home) + "/.config/hqc_config.db";
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", DB_CONFIG);
    db.setDatabaseName(dbPath);
    if (not db.open()) {
      fatalError(tr("Cannot access/create hqc config database, please check the HQC installation"), dbPath);
    }
  }
  return QSqlDatabase::database(DB_CONFIG);
}

QUrl HqcApplication::getKroUrl() const
{
  using namespace miutil::conf;
  const ValElementList valKroUrl = mConfig->getValue("kro.url");
  if (valKroUrl.size() == 1)
    return QUrl(QString::fromStdString(valKroUrl.front().valAsString()));
  else
    return QUrl("http://kro/cgi-bin/start.pl");
}

QSqlDatabase HqcApplication::kvalobsDB()
{
  return kvalobsDB(DB_KVALOBS);
}

QSqlDatabase HqcApplication::kvalobsDB(const QString& qname)
{
  if (not QSqlDatabase::contains(qname)) {
    if (not Helpers::connect2postgres(qname, mConfig, "kvalobsdb")) {
      fatalError(tr("Cannot access kvalobs SQL database, please check the HQC configuration"));
    }
  }
  return QSqlDatabase::database(qname);
}

QString HqcApplication::kvalobsDBName()
{
  const miutil::conf::ValElementList valHost = mConfig->getValue(std::string(CK_KVALOBSDB) + ".host");

  try {
    if (valHost.size() == 1)
      return Helpers::fromUtf8(valHost.front().valAsString());
  } catch (miutil::conf::InvalidTypeEx& e) {
    // pass
  }
  return QString("?");
}

void HqcApplication::setReinserter(AbstractReinserter_p reinserter)
{
  kda->setReinserter(reinserter);
}

void HqcApplication::saveLanguage(const QString& language)
{
  QSettings settings;
  if (settings.value(SETTING_HQC_LANGUAGE).toString() != language) {
    settings.setValue(SETTING_HQC_LANGUAGE, language);
    installTranslations();
  }
}

QString HqcApplication::savedLanguage() const
{
  QSettings settings;
  return settings.value(SETTING_HQC_LANGUAGE).toString();
}

QStringList HqcApplication::availableLanguages() const
{
  QStringList available("en");

  QDir langDir(::hqc::getPath(::hqc::LANGDIR));
  QStringList fileNames = langDir.entryList(QStringList("hqc_*.qm"));
  for (QString locale : fileNames) {
    //                                    locale = "hqc_de.qm"
    locale.truncate(locale.lastIndexOf('.'));   // "hqc_de"
    locale.remove(0, locale.indexOf('_') + 1);  // "de"
    available << locale;
  }

  return available;
}

void HqcApplication::installTranslations()
{
  for (QTranslator* t : mTranslators) {
    removeTranslator(t);
    delete t;
  }
  mTranslators.clear();

  QLocale locale = QLocale::system();
  QString sl = savedLanguage();
  if (not sl.isEmpty())
    locale = QLocale(sl);

  const QString langDir = ::hqc::getPath(::hqc::LANGDIR);
  // translators are searched in reverse order of installation
  installTranslations(locale, "qt", QLibraryInfo::location(QLibraryInfo::TranslationsPath));
  installTranslations(locale, "qUtilities", "/usr/share/metlibs/translations");
  const char* hqc_translations[] = {"common", "errorlist", "extremes", "hqc", "missingobs", "rejectedobs", "textdata", "util", " watchrr", "weather", 0};
  for (const char** t = hqc_translations; *t; ++t)
    installTranslations(locale, *t, langDir);
}

void HqcApplication::installTranslations(const QLocale& locale, const QString& file, const QStringList& paths)
{
  QTranslator* translator = new QTranslator(this);
  mTranslators.push_back(translator);

  for (const QString& p : paths) {
    if (translator->load(locale, file, "_", p)) {
      METLIBS_LOG_INFO("loaded '" << file << "' translations from " << p
          << " for ui languages=" << QLocale::system().uiLanguages().join(","));
      installTranslator(translator);
      return;
    }
  }
  METLIBS_LOG_INFO("failed to load '" << file << "' translations from ["
      << paths.join(",")
      << "] for ui languages=" << QLocale::system().uiLanguages().join(","));
}

bool HqcApplication::notify(QObject* receiver, QEvent* e)
{
  QString exception;
  try {
    return QApplication::notify(receiver, e);
  } catch (std::exception& e) {
    HQC_LOG_ERROR("exception in Qt event handling: " << e.what());
    onException(Helpers::fromUtf8(e.what()));
  } catch (...) {
    HQC_LOG_ERROR("unknown exception in Qt event handling");
    onException("");
  }
  return false;
}

void HqcApplication::onException(const QString& message)
{
  const bool isGuiThread = QThread::currentThread() == QCoreApplication::instance()->thread();
  if (isGuiThread) {
    QMessageBox w;
    w.setWindowTitle(tr("HQC"));
    w.setIcon(QMessageBox::Critical);
    w.setText(tr("A severe error has occurred. You should restart the application, and report the error."));
    if (not message.isEmpty())
      w.setInformativeText(message);

    w.setStandardButtons(QMessageBox::Close);
    w.setDefaultButton(QMessageBox::Close);
    w.exec();
    HQC_LOG_WARN("exception in gui thread: '" << message << "'");
  } else {
    HQC_LOG_WARN("exception in non-gui thread: '" << message << "'");
  }
}

bool HqcApplication::isGuiThread() const
{
  return (QCoreApplication::instance()
      and QThread::currentThread() == QCoreApplication::instance()->thread());
}

void HqcApplication::fatalError(const QString& message, const QString& info)
{
  if (isGuiThread()) {
    QMessageBox w;
    w.setWindowTitle(tr("HQC"));
    w.setIcon(QMessageBox::Critical);
    w.setText(message);
    if (not info.isEmpty())
      w.setInformativeText(info);

    w.setStandardButtons(QMessageBox::Abort);
    w.setDefaultButton(QMessageBox::Abort);
    w.exec();
  } else {
    HQC_LOG_ERROR("fatal error in non-gui thread: '" << message << "' with info '" + info + "'");
  }
  exit(-1);
}

void HqcApplication::exitNoKvalobs()
{
  METLIBS_LOG_SCOPE();
  if (isGuiThread()) {
    QMessageBox msg;
    msg.setIcon(QMessageBox::Critical);
    msg.setText(tr("The kvalobs database is not accessible."));
    msg.setInformativeText(tr("HQC terminates because it cannot be used without the kvalobs database."));
    msg.exec();
  } else {
    HQC_LOG_ERROR("kvalobs database not accessible in non-gui thread, exit");
  }
  exit(-1);
}

void HqcApplication::checkKvalobsAvailability()
{
  KvServiceHelper::instance()->checkKvalobsAvailability();
}

bool HqcApplication::isKvalobsAvailable() const
{
  return KvServiceHelper::instance()->isKvalobsAvailable();
}

void HqcApplication::changedKvalobsAvailability(bool available)
{
  /* emit Qt signal */ kvalobsAvailable(available);
}

int HqcApplication::exec()
{
  int r = QApplication::exec();
  if (r != 0)
    return r;
  if (HqcLoggingWarnOrError)
    return 2;
  return 0;
}
