
#ifndef ACCESS_SQLITEACCESS_HH
#define ACCESS_SQLITEACCESS_HH 1

#include "KvTypedefs.hh"
#include "QueryTaskAccess.hh"

#include <kvalobs/kvData.h>
#include <kvalobs/kvModelData.h>

class SqliteQueryRunner;

class SqliteAccess : public QueryTaskAccess
{ Q_OBJECT;
public:
  SqliteAccess(bool useThread = false);
  ~SqliteAccess();

  virtual ObsUpdate_p createUpdate(const SensorTime& sensorTime);
  virtual ObsUpdate_p createUpdate(ObsData_p data);
  virtual bool storeUpdates(const ObsUpdate_pv& updates);

  virtual void postRequest(ObsRequest_p request);
  virtual void dropRequest(ObsRequest_p request);

  size_t countPost() const
    { return mCountPost; }

  size_t countDrop() const
    { return mCountDrop; }

  // ==================== debugging interface ====================

  void insertDataFromFile(const std::string& filename);
  void insertData(const kvalobs::kvData& kvd);
  void dropData(const SensorTime_v& toDrop);

  void insertModelFromFile(const std::string& filename);
  void insertModel(const kvalobs::kvModelData& kvm);

  void insertObsPgm(const kvalobs::kvObsPgm& kvo);
  void insertParam(const kvalobs::kvParam& kvp);
  void insertStation(const kvalobs::kvStation& kvs);
  void insertTypes(const kvalobs::kvTypes& kvt);

  void execSQL(const std::string& sql);
  void clear();

  using QueryTaskAccess::handler;

private:
  boost::shared_ptr<SqliteQueryRunner> runner();

private:
  size_t mCountPost, mCountDrop;
};

HQC_TYPEDEF_P(SqliteAccess);

#endif // ACCESS_SQLITEACCESS_HH
