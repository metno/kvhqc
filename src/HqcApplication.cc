
#include "HqcApplication.hh"

#include "hqc_paths.hh"

#include <QtCore/QLibraryInfo>
#include <QtCore/QLocale>
#include <QtCore/QTranslator>

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
