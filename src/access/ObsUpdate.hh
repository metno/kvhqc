
#ifndef ACCESS_OBSUPDATE_HH
#define ACCESS_OBSUPDATE_HH 1

#include "ObsData.hh"

#include <vector>

/*! Observation update. */
class ObsUpdate : HQC_SHARED_NOCOPY(ObsUpdate) {
public:
  virtual ~ObsUpdate() { }

  /*! Observation sensor and time. */
  virtual SensorTime sensorTime() const = 0;
  
  /*! Current corrected value, modified during quality control. */
  virtual float corrected() const = 0;

  /*! KVALOBS current control flags. */
  virtual kvalobs::kvControlInfo controlinfo() const = 0;

  /*! KVALOBS current list of performed checks / modifications. */
  virtual std::string cfailed() const = 0;
};

HQC_TYPEDEF_P(ObsUpdate);
HQC_TYPEDEF_PV(ObsUpdate);

#endif // ACCESS_OBSUPDATE_HH
