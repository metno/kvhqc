
#ifndef ACCESS_KVALOBSACCESS_HH
#define ACCESS_KVALOBSACCESS_HH 1

#include "BackgroundAccess.hh"
#include "common/AbstractReinserter.hh"
#include "common/AbstractUpdateListener.hh"

#include <decodeutility/DataReinserter.h>
#include <kvcpp/KvApp.h>
#include <kvcpp/kvservicetypes.h>

class KvalobsThread;

class KvalobsAccess : public BackgroundAccess
{ Q_OBJECT;
public:
  KvalobsAccess();
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
  virtual void onUpdated(const kvData_v&);

private:
  void checkUnsubscribe(const Sensor_s& sensors);
  void checkSubscribe(const Sensor_s& sensors);

private:
  AbstractReinserter_p mDataReinserter;
};

HQC_TYPEDEF_P(KvalobsAccess);

#endif // ACCESS_KVALOBSACCESS_HH
