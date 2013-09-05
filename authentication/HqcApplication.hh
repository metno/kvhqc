
#ifndef HqcApplication_hh
#define HqcApplication_hh 1

#include <QtCore/QList>
#include <QtGui/QApplication>
#include <QtSql/QSqlDatabase>

class HqcApplication : public QApplication
{   Q_OBJECT;
public:
    HqcApplication(int & argc, char ** argv);
    ~HqcApplication();

//    void setReinserter(HqcReinserter* r, const QString& username)
//        { mw->setReinserter(r, username); }

    virtual bool notify(QObject* receiver, QEvent* e);

  QSqlDatabase systemDB();
  QSqlDatabase configDB();

private:
    void installTranslations(const QString& file, const QStringList& paths);
    inline void installTranslations(const QString& file, const QString& path)
        { installTranslations(file, QStringList(path)); }
    void onException(const QString& message);
    void fatalError(const QString& message, const QString& info="");

private:
    QList<QTranslator*> mTranslators;
};

extern HqcApplication* hqcApp;

#endif // HqcApplication_hh
