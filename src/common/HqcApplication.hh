
#ifndef HqcApplication_hh
#define HqcApplication_hh 1

#include "AbstractReinserter.hh"

#include <miconfparser/confsection.h>

#include <QtCore/QList>
#include <QtGui/QApplication>
#include <QtSql/QSqlDatabase>

#include <boost/shared_ptr.hpp>

class CachingAccess;
class EditAccess;
class HqcUserConfig;
class KvalobsAccess;
class ModelAccess;
class QueryTaskHandler;

class HqcApplication : public QApplication
{   Q_OBJECT;
public:
  HqcApplication(int & argc, char ** argv, miutil::conf::ConfSection* conf);
  ~HqcApplication();

  virtual bool notify(QObject* receiver, QEvent* e);

  QSqlDatabase systemDB();
  QSqlDatabase configDB();
  HqcUserConfig* userConfig()
    { return mUserConfig.get(); }

  boost::shared_ptr<QueryTaskHandler> kvalobsHandler() const
    { return mKvalobsHandler; }
  boost::shared_ptr<EditAccess> editAccess() const
    { return eda; }
  boost::shared_ptr<ModelAccess> modelAccess() const
    { return kma; }

  void setReinserter(AbstractReinserter_p reinserter);

  QSqlDatabase kvalobsDB();
  QSqlDatabase kvalobsDB(const QString& qname);

  void exitNoKvalobs();

  /** Query last known availability of kvServiced. Does not re-check. */
  bool isKvalobsAvailable() const;

  QString instanceName() const;

  int exec();

  void saveLanguage(const QString& language);
  QString savedLanguage() const;
  QStringList availableLanguages() const;

Q_SIGNALS:
  void kvalobsAvailable(bool);

private Q_SLOTS:
  void checkKvalobsAvailability();

private:
  void installTranslations();
  void installTranslations(const QLocale& locale, const QString& file, const QStringList& paths);
  inline void installTranslations(const QLocale& locale, const QString& file, const QString& path)
    { installTranslations(locale, file, QStringList(path)); }

  void onException(const QString& message);
  void fatalError(const QString& message, const QString& info="");
  bool isGuiThread() const;
  bool updateKvalobsAvailability(bool available);
  void changedKvalobsAvailability(bool);

private:
  QList<QTranslator*> mTranslators;
  miutil::conf::ConfSection *mConfig;
  bool mKvalobsAvailable;
  std::auto_ptr<HqcUserConfig> mUserConfig;

  boost::shared_ptr<QueryTaskHandler> mKvalobsHandler;
  boost::shared_ptr<KvalobsAccess> kda;
  boost::shared_ptr<CachingAccess> cda;
  boost::shared_ptr<EditAccess> eda;

  boost::shared_ptr<ModelAccess> kma;
};

extern HqcApplication* hqcApp;

#endif // HqcApplication_hh
