
#ifndef ACCESS_OBSACCESS_HH
#define ACCESS_OBSACCESS_HH 1

#include "ObsRequest.hh"
#include "ObsUpdate.hh"

#include <vector>

/*! Observation data buffer. */
class ObsAccess : HQC_SHARED_NOCOPY(ObsAccess) {
public:
  virtual ~ObsAccess();

  virtual void postRequest(ObsRequest_p request) = 0;

  /** Must be called before destroying the request, ie not from the ObsRequest destructor.
   */
  virtual void dropRequest(ObsRequest_p request) = 0;

  //! Create update for modification.
  virtual ObsUpdate_p createUpdate(ObsData_p data) = 0;

  //! Create update for insertion.
  virtual ObsUpdate_p createUpdate(const SensorTime& sensorTime) = 0;

  //! bool? exception? is it sync or not?
  virtual bool storeUpdates(const ObsUpdate_pv& updates) = 0;
};

HQC_TYPEDEF_P(ObsAccess);

#endif // ACCESS_OBSACCESS_HH
