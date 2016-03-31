
#include "HqcApplication.hh"

#include "common/HqcUserConfig.hh"
#include "common/KvServiceHelper.hh"
#include "common/TimeRange.hh"
#include "util/hqc_paths.hh"
#include "util/Helpers.hh"

#include <kvalobs/kvStationParam.h>
#include <kvcpp/KvApp.h>

#include <puTools/miString.h>

#include <QtCore/QDir>
#include <QtCore/QLibraryInfo>
#include <QtCore/QLocale>
#include <QtCore/QSettings>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtCore/QTranslator>
#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtGui/QMessageBox>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.HqcApplication"
#define M_TIME
#include "util/HqcLogging.hh"

namespace /* anonymous */ {
const char DB_SYSTEM[] = "hqc_system_db";
const char DB_CONFIG[] = "hqc_config_db";
const char DB_KVALOBS[] = "kvalobs_db";

const int AVAILABILITY_TIMEROUT = 120*1000; // milliseconds = 2 minutes

const char SETTING_HQC_LANGUAGE[] = "language";
} // anonymous namespace

HqcApplication* hqcApp = 0;

HqcApplication::HqcApplication(int & argc, char ** argv, miutil::conf::ConfSection *conf)
  : QApplication(argc, argv)
  , mConfig(conf)
{
  hqcApp = this;

  setlocale(LC_NUMERIC, "C");

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
  
  KvServiceHelper::instance()->kvalobsAvailable.connect(boost::bind(&HqcApplication::changedKvalobsAvailability, this, _1));
}

HqcApplication::~HqcApplication()
{
  KvServiceHelper::instance()->kvalobsAvailable.disconnect(boost::bind(&HqcApplication::changedKvalobsAvailability, this, _1));

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

    const QString dbPath = QString::fromStdString(home) + "/.config/hqc_config.db";
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", DB_CONFIG);
    db.setDatabaseName(dbPath);
    if (not db.open()) {
      fatalError(tr("Cannot access/create hqc config database, please check the HQC installation"), dbPath);
    }
  }
  return QSqlDatabase::database(DB_CONFIG);
}

QSqlDatabase HqcApplication::kvalobsDB()
{
  if (not QSqlDatabase::contains(DB_KVALOBS)) {
    if (not Helpers::connect2postgres(DB_KVALOBS, mConfig, "kvalobsdb")) {
      fatalError(tr("Cannot access kvalobs SQL database, please check the HQC configuration"));
    }
  }
  return QSqlDatabase::database(DB_KVALOBS);
}

std::string HqcApplication::kvalobsColumnsSensorTime(const std::string& data_alias)
{
  std::stringstream columns;
  const std::string d = data_alias.empty() ? "" : (data_alias + ".");
  columns << d << "stationid,"
          << d << "paramid,"
          << d << "level,"
          << d << "sensor,"
          << d << "typeid,"
          << d << "obstime";
  return columns.str();
}

std::vector<SensorTime> HqcApplication::kvalobsQuerySensorTime(const std::string& sql)
{
  METLIBS_LOG_TIME();
  METLIBS_LOG_DEBUG(sql);

  QSqlQuery query(hqcApp->kvalobsDB());
  std::vector<SensorTime> results;
  if (query.exec(QString::fromStdString(sql))) {
    while (query.next()) {
      const Sensor s(query.value(0).toInt(), query.value(1).toInt(), query.value(2).toInt(),
          query.value(3).toInt(), query.value(4).toInt());
      const timeutil::ptime t = timeutil::from_iso_extended_string(query.value(5).toString().toStdString());
      const SensorTime st(s, t);
      results.push_back(st);
    }
  } else {
    HQC_LOG_ERROR("query '" << sql << "' failed: " << query.lastError().text());
  }
  return results;
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

  QDir langDir(::hqc::getPath(::hqc::DATADIR) + "/lang");
  QStringList fileNames = langDir.entryList(QStringList("hqc_*.qm"));
  Q_FOREACH(QString locale, fileNames) {
    //                                    locale = "hqc_de.qm"
    locale.truncate(locale.lastIndexOf('.'));   // "hqc_de"
    locale.remove(0, locale.indexOf('_') + 1);  // "de"
    available << locale;
  }

  return available;
}

void HqcApplication::installTranslations()
{
  BOOST_FOREACH(QTranslator* t, mTranslators) {
    removeTranslator(t);
    delete t;
  }
  mTranslators.clear();

  QLocale locale = QLocale::system();
  QString sl = savedLanguage();
  if (not sl.isEmpty())
    locale = QLocale(sl);

  const QString langDir = ::hqc::getPath(::hqc::DATADIR) + "/lang";
  // translators are searched in reverse order of installation
  installTranslations(locale, "qt", QLibraryInfo::location(QLibraryInfo::TranslationsPath));
  installTranslations(locale, "qUtilities", "/usr/share/metlibs/translations");
  installTranslations(locale, "watchrr", langDir);
  installTranslations(locale, "weather", langDir);
  installTranslations(locale, "hqc",     langDir);
}

void HqcApplication::installTranslations(const QLocale& locale, const QString& file, const QStringList& paths)
{
  QTranslator* translator = new QTranslator(this);
  mTranslators.push_back(translator);
  
  BOOST_FOREACH(const QString& p, paths) {
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
    onException(QString::fromStdString(e.what()));
  } catch (...) {
    HQC_LOG_ERROR("unknown exception in Qt event handling");
    onException("");
  }
  return false;
}

void HqcApplication::onException(const QString& message)
{
  const bool isGuiThread = 
      QThread::currentThread() == QCoreApplication::instance()->thread();
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
