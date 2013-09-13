
#ifndef COMMON_OBSDATA_HH
#define COMMON_OBSDATA_HH 1

#include "Sensor.hh"

#include <kvalobs/flag/kvControlInfo.h>

#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

/*! Observation data. */
class ObsData : public boost::enable_shared_from_this<ObsData>, private boost::noncopyable {
public:
  virtual ~ObsData() { }

  /*! Observation sensor and time. */
  virtual SensorTime sensorTime() const = 0;
  
  /*! Original observed value, as sent by the station. */
  virtual float original() const = 0;

  /*! Current corrected value, modified during quality control. */
  virtual float corrected() const = 0;

  /*! KVALOBS current control flags. */
  virtual kvalobs::kvControlInfo controlinfo() const = 0;

  /*! KVALOBS current list of performed checks / modifications. */
  virtual std::string cfailed() const = 0;
};
typedef boost::shared_ptr<ObsData> ObsDataPtr;

#endif // COMMON_OBSDATA_HH
