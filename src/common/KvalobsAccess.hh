
#ifndef ACCESS_KVALOBSACCESS_HH
#define ACCESS_KVALOBSACCESS_HH 1

#include "AbstractReinserter.hh"
#include "KvTypedefs.hh"
#include "QueryTaskAccess.hh"

class KvalobsAccess : public QueryTaskAccess
{ Q_OBJECT;
public:
  KvalobsAccess(QueryTaskHandler_p handler);
  ~KvalobsAccess();

  virtual void postRequest(ObsRequest_p request);
  virtual void dropRequest(ObsRequest_p request);

  virtual ObsUpdate_p createUpdate(ObsData_p data);
  virtual ObsUpdate_p createUpdate(const SensorTime& sensorTime);
  virtual bool storeUpdates(const ObsUpdate_pv& updates);

  void setReinserter(AbstractReinserter_p reinserter)
    { mDataReinserter = reinserter; }

  bool hasReinserter() const
    { return mDataReinserter; }

private Q_SLOTS:
  virtual void onUpdated(const hqc::kvData_v&);

private:
  void checkUnsubscribe(const Sensor_s& sensors);
  void checkSubscribe(const Sensor_s& sensors);

private:
  AbstractReinserter_p mDataReinserter;
};

HQC_TYPEDEF_P(KvalobsAccess);

#endif // ACCESS_KVALOBSACCESS_HH
