
#include "HqcApplication.hh"

#include "hqc_paths.hh"

#include <puTools/miString.h>

#include <QtCore/QLibraryInfo>
#include <QtCore/QLocale>
#include <QtCore/QThread>
#include <QtCore/QTranslator>
#include <QtGui/QMessageBox>

#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.HqcApplication"
#include "HqcLogging.hh"

namespace /* anonymous */ {
const char DB_SYSTEM[] = "hqc_system_db";
const char DB_CONFIG[] = "hqc_config_db";
} // anonymous namespace

HqcApplication* hqcApp = 0;

HqcApplication::HqcApplication(int & argc, char ** argv)
    : QApplication(argc, argv)
{
    hqcApp = this;

    QCoreApplication::setOrganizationName("Meteorologisk Institutt");
    QCoreApplication::setOrganizationDomain("met.no");
    QCoreApplication::setApplicationName("Hqc");

    installTranslations("qt", QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    installTranslations("qUtilities", "/usr/share/metlibs/translations");
    
    const QString langDir = ::hqc::getPath(::hqc::DATADIR) + "/lang";
    installTranslations("watchrr2", langDir);
    installTranslations("hqc",      langDir);

    connect(this, SIGNAL(lastWindowClosed()), this, SLOT(quit()));
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
      // not reached
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
      // not reached
    }
  }
  return QSqlDatabase::database(DB_CONFIG);
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

void HqcApplication::fatalError(const QString& message, const QString& info)
{
  const bool isGuiThread = 
      QThread::currentThread() == QCoreApplication::instance()->thread();
  if (isGuiThread) {
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
