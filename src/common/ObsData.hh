
#ifndef OBSDATA_HH
#define OBSDATA_HH 1

#include "Sensor.hh"

#include <kvalobs/flag/kvControlInfo.h>

#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

class ObsData : public boost::enable_shared_from_this<ObsData>, private boost::noncopyable {
public:
    virtual ~ObsData() { }
    virtual SensorTime sensorTime() const = 0;
    virtual float original() const = 0;
    virtual float corrected() const = 0;
    virtual kvalobs::kvControlInfo controlinfo() const = 0;
    virtual std::string cfailed() const = 0;
};
typedef boost::shared_ptr<ObsData> ObsDataPtr;

#endif // OBSDATA_HH
