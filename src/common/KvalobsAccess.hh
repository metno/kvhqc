
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

  void postRequest(ObsRequest_p request, bool synchronized=false) Q_DECL_OVERRIDE;
  void dropRequest(ObsRequest_p request) Q_DECL_OVERRIDE;

  ObsUpdate_p createUpdate(ObsData_p data) Q_DECL_OVERRIDE;
  ObsUpdate_p createUpdate(const SensorTime& sensorTime) Q_DECL_OVERRIDE;
  bool storeUpdates(const ObsUpdate_pv& updates) Q_DECL_OVERRIDE;

  void setReinserter(AbstractReinserter_p reinserter)
    { mDataReinserter = reinserter; }

  bool hasReinserter() const
    { return (bool)mDataReinserter; }

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
