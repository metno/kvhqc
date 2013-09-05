
#ifndef HqcApplication_hh
#define HqcApplication_hh 1

#include <miconfparser/confsection.h>

#include <QtCore/QList>
#include <QtGui/QApplication>
#include <QtSql/QSqlDatabase>

class HqcApplication : public QApplication
{   Q_OBJECT;
public:
  HqcApplication(int & argc, char ** argv, miutil::conf::ConfSection* conf);
    ~HqcApplication();

//    void setReinserter(HqcReinserter* r, const QString& username)
//        { mw->setReinserter(r, username); }

    virtual bool notify(QObject* receiver, QEvent* e);

  QSqlDatabase systemDB();
  QSqlDatabase configDB();
  QSqlDatabase kvalobsDB();

  void exitNoKvalobs();

  /** Query last known availability of kvServiced. Does not re-check. */
  bool isKvalobsAvailable() const;

  int exec();

  void setReturnCode(int r)
    { mReturnCode = r; }

Q_SIGNALS:
  void kvalobsAvailable(bool);

private Q_SLOTS:
  void checkKvalobsAvailability();

private:
  void installTranslations(const QString& file, const QStringList& paths);
  inline void installTranslations(const QString& file, const QString& path)
    { installTranslations(file, QStringList(path)); }
  void onException(const QString& message);
  void fatalError(const QString& message, const QString& info="");
  bool isGuiThread() const;
  bool updateKvalobsAvailability(bool available);
  void changedKvalobsAvailability(bool);

private:
  QList<QTranslator*> mTranslators;
  miutil::conf::ConfSection *mConfig;
  bool mKvalobsAvailable;
  int mReturnCode;
};

extern HqcApplication* hqcApp;

#endif // HqcApplication_hh
