
#ifndef HqcApplication_hh
#define HqcApplication_hh 1

#include "CachingAccess.hh"
#include "EditAccess.hh"
#include "KvalobsAccess.hh"
#include "KvalobsModelAccess.hh"

#include <miconfparser/confsection.h>

#include <QtCore/QList>
#include <QtGui/QApplication>
#include <QtSql/QSqlDatabase>

class CachingAccess;
class EditAccess;
class HqcUserConfig;
class KvalobsAccess;
class KvalobsModelAccess;

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

  EditAccess_p editAccess() const
    { return eda; }
  ModelAccess_p modelAccess() const
    { return kma; }

  QSqlDatabase kvalobsDB();
  QSqlDatabase kvalobsDB(const QString& qname);
  std::string kvalobsColumnsSensorTime(const std::string& data_alias="");
  std::vector<SensorTime> kvalobsQuerySensorTime(const std::string& constraint);

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

  KvalobsAccess_p kda;
  CachingAccess_p cda;
  EditAccess_p eda;

  ModelAccess_p kma;
};

extern HqcApplication* hqcApp;

#endif // HqcApplication_hh
