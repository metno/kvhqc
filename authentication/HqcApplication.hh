
#ifndef HqcApplication_hh
#define HqcApplication_hh 1

#include <miconfparser/confsection.h>

#include <QtCore/QList>
#include <QtGui/QApplication>
#include <QtSql/QSqlDatabase>

class TimeRange;
namespace kvalobs {
class kvModelData;
class kvRejectdecode;
class kvParam;
class kvStation;
class kvTypes;
class kvObsPgm;
class kvOperator;
}
namespace kvservice {
class KvGetDataReceiver;
class WhichDataHelper;
}

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
  bool isKvalobsAvailable() const
    { return mKvalobsAvailable; }

  bool getKvData(kvservice::KvGetDataReceiver& dataReceiver, const kvservice::WhichDataHelper& wd);
  bool getKvModelData(std::list<kvalobs::kvModelData> &dataList, const kvservice::WhichDataHelper& wd);
  bool getKvRejectDecode(std::list<kvalobs::kvRejectdecode>& rejectList, const TimeRange& timeLimits);
  bool getKvParams(std::list<kvalobs::kvParam>& paramList);
  bool getKvStations( std::list<kvalobs::kvStation>& stationList);
  bool getKvTypes(std::list<kvalobs::kvTypes>& typeList);
  bool getKvObsPgm(std::list<kvalobs::kvObsPgm>& obsPgm, const std::list<long>& stationList);
  bool getKvOperator(std::list<kvalobs::kvOperator>& operatorList );

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

private:
  QList<QTranslator*> mTranslators;
  miutil::conf::ConfSection *mConfig;
  bool mKvalobsAvailable;
};

extern HqcApplication* hqcApp;

#endif // HqcApplication_hh
