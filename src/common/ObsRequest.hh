
#ifndef ACCESS_OBSREQUEST_HH
#define ACCESS_OBSREQUEST_HH 1

#include "ObsData.hh"
#include "ObsFilter.hh"
#include "TimeSpan.hh"
#include "util/TaggedObject.hh"
#include <QObject>

#ifndef Q_DECL_OVERRIDE
#define Q_DECL_OVERRIDE override
#endif

// ========================================================================

/*! Observation data request. */
class ObsRequest : public QObject, public TaggedObject
{ Q_OBJECT;
public:
  ObsRequest();
  virtual ~ObsRequest();
  
  virtual const Sensor_s& sensors() const = 0;
  virtual const TimeSpan& timeSpan() const = 0;
  virtual ObsFilter_p filter() const = 0;

public:
  virtual void completed(const QString& withError);
  virtual void newData(const ObsData_pv& data) = 0;
  virtual void updateData(const ObsData_pv& data) = 0;
  virtual void dropData(const SensorTime_v& dropped) = 0;

Q_SIGNALS:
  void requestCompleted(const QString& withError);
};

HQC_TYPEDEF_P(ObsRequest);
HQC_TYPEDEF_PV(ObsRequest);

#endif // ACCESS_OBSREQUEST_HH
