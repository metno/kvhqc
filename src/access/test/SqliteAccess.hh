
#ifndef ACCESS_SQLITEACCESS_HH
#define ACCESS_SQLITEACCESS_HH 1

#include "ObsAccess.hh"
#include <QtCore/QSemaphore>
#include <string>

class SqliteHandler;
HQC_TYPEDEF_P(SqliteHandler);

class SqliteThread;

class SqliteAccess : public QObject, public ObsAccess
{ Q_OBJECT;
public:
  SqliteAccess(bool useThread = false);
  ~SqliteAccess();

  virtual void postRequest(ObsRequest_p request);
  virtual void dropRequest(ObsRequest_p request);

  virtual ObsUpdate_p createUpdate(const SensorTime& sensorTime);
  virtual bool storeUpdates(const ObsUpdate_pv& updates);

  void insertDataFromFile(const std::string& filename);

  size_t countPost() const
    { return mCountPost; }

  size_t countDrop() const
    { return mCountDrop; }

private Q_SLOTS:
  void onNewData(ObsRequest_p request, const ObsData_pv& data);

  void checkUpdates();
  void resubscribe();

private:
  boost::shared_ptr<SqliteHandler> mSqlite;
  size_t mCountPost, mCountDrop;

  SqliteThread* mThread;

  ObsRequest_pv mRequests;
  
  typedef std::map<int, int> stationid_count_m;
  stationid_count_m mStationCounts;
};

HQC_TYPEDEF_P(SqliteAccess);

#endif // ACCESS_SQLITEACCESS_HH
