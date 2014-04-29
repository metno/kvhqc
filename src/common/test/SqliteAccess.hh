
#ifndef ACCESS_SQLITEACCESS_HH
#define ACCESS_SQLITEACCESS_HH 1

#include "BackgroundAccess.hh"

class SqliteAccess : public BackgroundAccess
{ Q_OBJECT;
public:
  SqliteAccess(bool useThread = false);
  ~SqliteAccess();

  virtual ObsUpdate_p createUpdate(const SensorTime& sensorTime);
  virtual ObsUpdate_p createUpdate(ObsData_p data);
  virtual bool storeUpdates(const ObsUpdate_pv& updates);

  void insertDataFromFile(const std::string& filename);
  void dropData(const SensorTime_v& toDrop);

  virtual void postRequest(ObsRequest_p request);
  virtual void dropRequest(ObsRequest_p request);

  size_t countPost() const
    { return mCountPost; }

  size_t countDrop() const
    { return mCountDrop; }

private:
  size_t mCountPost, mCountDrop;
};

HQC_TYPEDEF_P(SqliteAccess);

#endif // ACCESS_SQLITEACCESS_HH
