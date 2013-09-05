
#include "HqcApplication.hh"

#include "hqc_paths.hh"
#include "hqc_utilities.hh"
#include "KvServiceHelper.hh"
#include "TimeRange.hh"

#include <kvalobs/kvStationParam.h>
#include <kvcpp/KvApp.h>

#include <puTools/miString.h>

#include <QtCore/QDir>
#include <QtCore/QLibraryInfo>
#include <QtCore/QLocale>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtCore/QTranslator>
#include <QtGui/QIcon>
#include <QtGui/QMessageBox>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.HqcApplication"
#include "HqcLogging.hh"

namespace /* anonymous */ {
const char DB_SYSTEM[] = "hqc_system_db";
const char DB_CONFIG[] = "hqc_config_db";
const char DB_KVALOBS[] = "kvalobs_db";

const int AVAILABILITY_TIMEROUT = 120*1000; // milliseconds = 2 minutes
} // anonymous namespace

HqcApplication* hqcApp = 0;

HqcApplication::HqcApplication(int & argc, char ** argv, miutil::conf::ConfSection *conf)
    : QApplication(argc, argv)
    , mConfig(conf)
{
    hqcApp = this;

    QCoreApplication::setOrganizationName("Meteorologisk Institutt");
    QCoreApplication::setOrganizationDomain("met.no");
    QCoreApplication::setApplicationName("Hqc");

    installTranslations("qt", QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    installTranslations("qUtilities", "/usr/share/metlibs/translations");
    
    const QString langDir = ::hqc::getPath(::hqc::DATADIR) + "/lang";
    installTranslations("watchrr2", langDir);
    installTranslations("weather",  langDir);
    installTranslations("hqc",      langDir);

    QDir::setSearchPaths("icons", QStringList(hqc::getPath(hqc::IMAGEDIR)));

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

void HqcApplication::installTranslations(const QString& file, const QStringList& paths)
{
    QTranslator* translator = new QTranslator(this);
    mTranslators.push_back(translator);
    
    BOOST_FOREACH(const QString& p, paths)
        if (translator->load(QLocale::system(), file, "_", p)) {
            METLIBS_LOG_INFO("loaded '" << file << "' translations from " << p
                         << " for ui languages=" << QLocale::system().uiLanguages().join(","));
            installTranslator(translator);
            return;
        }
    METLIBS_LOG_WARN("failed to load '" << file << "' translations from [" 
                 << paths.join(",")
                 << "] for ui languages=" << QLocale::system().uiLanguages().join(","));
}

bool HqcApplication::notify(QObject* receiver, QEvent* e)
{
  QString exception;
  try {
    return QApplication::notify(receiver, e);
  } catch (std::exception& e) {
    METLIBS_LOG_ERROR("exception in Qt event handling: " << e.what());
    onException(QString::fromStdString(e.what()));
  } catch (...) {
    METLIBS_LOG_ERROR("unknown exception in Qt event handling");
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
  } else {
    METLIBS_LOG_WARN("exception in non-gui thread: '" << message << "'");
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
    METLIBS_LOG_ERROR("fatal error in non-gui thread: '" << message << "' with info '" + info + "'");
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
    METLIBS_LOG_ERROR("kvalobs database not accessible in non-gui thread, exit");
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
