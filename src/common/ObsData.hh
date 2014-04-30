
#ifndef ACCESS_OBSDATA_HH
#define ACCESS_OBSDATA_HH 1

#include "common/Sensor.hh"

#include <kvalobs/flag/kvControlInfo.h>

#include "util/boostutil.hh"

/*! Observation data. */
class ObsData : HQC_SHARED_NOCOPY(ObsData) {
public:
  virtual ~ObsData() { }

  /*! Observation sensor and time. */
  virtual const SensorTime& sensorTime() const = 0;
  
  /*! Original observed value, as sent by the station. */
  virtual float original() const = 0;

  /*! Current corrected value, modified during quality control. */
  virtual float corrected() const = 0;

  /*! KVALOBS current control flags. */
  virtual const kvalobs::kvControlInfo& controlinfo() const = 0;

  /*! KVALOBS current list of performed checks / modifications. */
  virtual const std::string& cfailed() const = 0;

  /*! KVALOBS tbtime. */
  virtual const timeutil::ptime& tbtime() const = 0;

  virtual bool isModified() const
    { return false; }
};

HQC_TYPEDEF_P(ObsData);
HQC_TYPEDEF_PV(ObsData);
HQC_TYPEDEF_PS(ObsData);
HQC_TYPEDEF_PL(ObsData);

#endif // ACCESS_OBSDATA_HH
