
#ifndef ACCESS_KVALOBSACCESS_HH
#define ACCESS_KVALOBSACCESS_HH 1

#include "BackgroundAccess.hh"
#include "common/AbstractUpdateListener.hh"

class KvalobsThread;

class KvalobsAccess : public BackgroundAccess
{ Q_OBJECT;
public:
  KvalobsAccess();
  ~KvalobsAccess();

  virtual void postRequest(ObsRequest_p request);
  virtual void dropRequest(ObsRequest_p request);

  virtual ObsUpdate_p createUpdate(const SensorTime& sensorTime);
  virtual bool storeUpdates(const ObsUpdate_pv& updates);

private Q_SLOTS:
  virtual void onUpdated(const kvData_v&);

private:
  void checkUnsubscribe(const Sensor_s& sensors);
  void checkSubscribe(const Sensor_s& sensors);
};

HQC_TYPEDEF_P(KvalobsAccess);

#endif // ACCESS_KVALOBSACCESS_HH
