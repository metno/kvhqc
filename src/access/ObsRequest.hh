
#ifndef ACCESS_OBSREQUEST_HH
#define ACCESS_OBSREQUEST_HH 1

#include "ObsData.hh"
#include "ObsFilter.hh"
#include "TimeSpan.hh"

#include "boostutil.hh"

#include <vector>
HQC_TYPEDEF_V(SensorTime);

// ========================================================================

/*! Observation data request. */
class ObsRequest : HQC_SHARED_NOCOPY(ObsRequest)
{
public:
  ObsRequest();
  virtual ~ObsRequest();
  
  //const Sensor_s& sensors() const = 0;
  virtual const Sensor& sensor() const = 0;

  virtual const TimeSpan& timeSpan() const = 0;

  virtual ObsFilter_p filter() const = 0;

  /** Tag is set by the ObsAccess instance that handles this request. */
  typedef void* Tag;

  Tag setTag(Tag tag)
    { std::swap(mTag, tag); return tag; }

  Tag tag() const
    { return mTag; }

public:
  virtual void completed(bool failed) = 0;
  virtual void newData(const ObsData_pv& data) = 0;
  virtual void updateData(const ObsData_pv& data) = 0;
  virtual void dropData(const SensorTime_v& dropped) = 0;

private:
  Tag mTag;
};

HQC_TYPEDEF_P(ObsRequest);
HQC_TYPEDEF_PV(ObsRequest);

#endif // ACCESS_OBSREQUEST_HH
