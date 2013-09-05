
#include "HqcApplication.hh"

#include "hqc_paths.hh"

#include <QtCore/QLibraryInfo>
#include <QtCore/QLocale>
#include <QtCore/QThread>
#include <QtCore/QTranslator>
#include <QtGui/QMessageBox>

#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.HqcApplication"
#include "HqcLogging.hh"

HqcApplication::HqcApplication(int & argc, char ** argv)
    : QApplication(argc, argv)
{
    QCoreApplication::setOrganizationName("Meteorologisk Institutt");
    QCoreApplication::setOrganizationDomain("met.no");
    QCoreApplication::setApplicationName("Hqc");

    installTranslations("qt", QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    installTranslations("qUtilities", "/usr/share/metlibs/translations");
    
    const QString langDir = ::hqc::getPath(::hqc::DATADIR) + "/lang";
    installTranslations("watchrr2", langDir);
    installTranslations("hqc",      langDir);

    mw = std::auto_ptr<HqcMainWindow>(new HqcMainWindow());
    setMainWidget(mw.get());

    connect(this, SIGNAL(lastWindowClosed()), this, SLOT(quit()));
}

HqcApplication::~HqcApplication()
{
}

void HqcApplication::startup(const QString& captionSuffix)
{
    mw->setCaption("HQC " + captionSuffix);
    mw->startup();
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
