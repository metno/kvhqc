
#ifndef ACCESS_CACHINGACCESS_HH
#define ACCESS_CACHINGACCESS_HH 1

#include "ObsAccess.hh"

class CachingAccessPrivate;
HQC_TYPEDEF_P(CachingAccessPrivate);

class CachingAccess : public ObsAccess
{
public:
  CachingAccess(ObsAccess_p backend);
  ~CachingAccess();

  virtual void postRequest(ObsRequest_p request, bool synchronized=false);

  virtual void dropRequest(ObsRequest_p request);

  virtual ObsUpdate_p createUpdate(ObsData_p data);
  virtual ObsUpdate_p createUpdate(const SensorTime& sensorTime);
  virtual bool storeUpdates(const ObsUpdate_pv& updates);

  void cleanCache(const Time& maxAge);

private:
  CachingAccessPrivate_p p;
};

HQC_TYPEDEF_P(CachingAccess);

#endif // ACCESS_CACHINGACCESS_HH
