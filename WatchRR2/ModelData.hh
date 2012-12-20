
#ifndef MODELDATA_HH
#define MODELDATA_HH 1

#include "Sensor.hh"

#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

class ModelData : public boost::enable_shared_from_this<ModelData>, private boost::noncopyable {
public:
    virtual ~ModelData() { }
    virtual SensorTime sensorTime() const = 0;
    virtual float value() const = 0;
};
typedef boost::shared_ptr<ModelData> ModelDataPtr;

#endif // MODELDATA_HH
