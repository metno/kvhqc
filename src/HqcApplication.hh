
#ifndef HqcApplication_hh
#define HqcApplication_hh 1

#include "hqcmain.h"

#include <QtCore/QList>
#include <QtGui/QApplication>

#include <memory>

class HqcApplication : public QApplication
{   Q_OBJECT;
public:
    HqcApplication(int & argc, char ** argv);
    ~HqcApplication();

    void startup(const QString& captionSuffix);
    void setReinserter(HqcReinserter* r, const QString& username)
        { mw->setReinserter(r, username); }

private:
    void installTranslations(const QString& file, const QStringList& paths);
    inline void installTranslations(const QString& file, const QString& path)
        { installTranslations(file, QStringList(path)); }

private:
    std::auto_ptr<HqcMainWindow> mw;

    QList<QTranslator*> mTranslators;
};

#endif // HqcApplication_hh
