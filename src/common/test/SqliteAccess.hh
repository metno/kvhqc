
#ifndef ACCESS_SQLITEACCESS_HH
#define ACCESS_SQLITEACCESS_HH 1

#include "common/KvTypedefs.hh"
#include "common/QueryTaskAccess.hh"
#include "util/hqcObsPgm.h"

class SqliteQueryRunner;

class SqliteAccess : public QueryTaskAccess
{ Q_OBJECT
public:
  SqliteAccess(bool useThread = false);
  ~SqliteAccess();

  ObsUpdate_p createUpdate(const SensorTime& sensorTime) Q_DECL_OVERRIDE;
  ObsUpdate_p createUpdate(ObsData_p data) Q_DECL_OVERRIDE;
  bool storeUpdates(const ObsUpdate_pv& updates) Q_DECL_OVERRIDE;

  void postRequest(ObsRequest_p request, bool synchronized=false) Q_DECL_OVERRIDE;
  void dropRequest(ObsRequest_p request) Q_DECL_OVERRIDE;

  size_t countPost() const
    { return mCountPost; }

  size_t countDrop() const
    { return mCountDrop; }

  // ==================== debugging interface ====================

  void insertDataFromFile(const std::string& filename);
  void insertDataFromText(const std::string& line, const timeutil::ptime& tbtime = timeutil::now());
  void insertData(const kvalobs::kvData& kvd);
  void dropData(const SensorTime_v& toDrop);

  void insertModelFromFile(const std::string& filename);
  void insertModel(const kvalobs::kvModelData& kvm);

  void insertObsPgm(const hqc::hqcObsPgm& kvo);
  void insertParam(const kvalobs::kvParam& kvp);
  void insertStation(const kvalobs::kvStation& kvs);
  void insertTypes(const kvalobs::kvTypes& kvt);

  void execSQL(const std::string& sql);
  void clear();

  using QueryTaskAccess::handler;

private:
  std::shared_ptr<SqliteQueryRunner> runner();

private:
  size_t mCountPost, mCountDrop;
};

HQC_TYPEDEF_P(SqliteAccess);

#endif // ACCESS_SQLITEACCESS_HH
