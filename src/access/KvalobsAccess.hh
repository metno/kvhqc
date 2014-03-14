
#ifndef ACCESS_KVALOBSACCESS_HH
#define ACCESS_KVALOBSACCESS_HH 1

#include "BackgroundAccess.hh"

class KvalobsThread;

class KvalobsAccess : public BackgroundAccess
{
public:
  KvalobsAccess();
  ~KvalobsAccess();

  virtual void postRequest(ObsRequest_p request);
  virtual void dropRequest(ObsRequest_p request);

  virtual ObsUpdate_p createUpdate(const SensorTime& sensorTime);
  virtual bool storeUpdates(const ObsUpdate_pv& updates);

private:
  void checkUnsubscribe(const Sensor_s& sensors);
  void checkSubscribe(const Sensor_s& sensors);
  void resubscribe();

private:
  typedef std::map<int, int> stationid_count_m;
  stationid_count_m mStationCounts;
};

HQC_TYPEDEF_P(KvalobsAccess);

#endif // ACCESS_KVALOBSACCESS_HH
