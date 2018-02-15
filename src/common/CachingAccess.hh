
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

  void postRequest(ObsRequest_p request) Q_DECL_OVERRIDE;

  void dropRequest(ObsRequest_p request) Q_DECL_OVERRIDE;

  ObsUpdate_p createUpdate(ObsData_p data) Q_DECL_OVERRIDE;
  ObsUpdate_p createUpdate(const SensorTime& sensorTime) Q_DECL_OVERRIDE;
  bool storeUpdates(const ObsUpdate_pv& updates) Q_DECL_OVERRIDE;

  void cleanCache(const Time& maxAge);

private:
  CachingAccessPrivate_p p;
};

HQC_TYPEDEF_P(CachingAccess);

#endif // ACCESS_CACHINGACCESS_HH
