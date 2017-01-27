
#ifndef HqcApplication_hh
#define HqcApplication_hh 1

#include "AbstractReinserter.hh"

#include <miconfparser/confsection.h>

#include <QApplication>
#include <QList>
#include <QStringList>
#include <QSqlDatabase>

#include <memory>

class CachingAccess;
class EditAccess;
class HqcUserConfig;
class KvalobsAccess;
class ModelAccess;
class QueryTaskHandler;

class HqcApplication : public QApplication
{   Q_OBJECT;
public:
  HqcApplication(int & argc, char ** argv, std::shared_ptr<miutil::conf::ConfSection>);
  ~HqcApplication();

  virtual bool notify(QObject* receiver, QEvent* e);

  QSqlDatabase systemDB();
  QSqlDatabase configDB();
  HqcUserConfig* userConfig()
    { return mUserConfig.get(); }

  std::shared_ptr<QueryTaskHandler> kvalobsHandler() const
    { return mKvalobsHandler; }
  std::shared_ptr<EditAccess> editAccess() const
    { return eda; }
  std::shared_ptr<ModelAccess> modelAccess() const
    { return kma; }

  void setReinserter(AbstractReinserter_p reinserter);

  QSqlDatabase kvalobsDB();
  QSqlDatabase kvalobsDB(const QString& qname);
  QString kvalobsDBName();

  void exitNoKvalobs();

  /** Query last known availability of kvServiced. Does not re-check. */
  bool isKvalobsAvailable() const;

  int exec();

  void saveLanguage(const QString& language);
  QString savedLanguage() const;
  QStringList availableLanguages() const;

Q_SIGNALS:
  void kvalobsAvailable(bool);

private Q_SLOTS:
  void checkKvalobsAvailability();
  void changedKvalobsAvailability(bool);

private:
  void installTranslations();
  void installTranslations(const QLocale& locale, const QString& file, const QStringList& paths);
  inline void installTranslations(const QLocale& locale, const QString& file, const QString& path)
    { installTranslations(locale, file, QStringList(path)); }

  void onException(const QString& message);
  void fatalError(const QString& message, const QString& info="");
  bool isGuiThread() const;
  bool updateKvalobsAvailability(bool available);

private:
  QList<QTranslator*> mTranslators;
  std::shared_ptr<miutil::conf::ConfSection> mConfig;
  bool mKvalobsAvailable;
  std::unique_ptr<HqcUserConfig> mUserConfig;

  std::shared_ptr<QueryTaskHandler> mKvalobsHandler;
  std::shared_ptr<KvalobsAccess> kda;
  std::shared_ptr<CachingAccess> cda;
  std::shared_ptr<EditAccess> eda;

  std::shared_ptr<ModelAccess> kma;
};

extern HqcApplication* hqcApp;

#endif // HqcApplication_hh
