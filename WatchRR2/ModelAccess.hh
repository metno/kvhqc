
#ifndef ModelAccess_hh
#define ModelAccess_hh 1

#include "Sensor.hh"
#include <boost/signals.hpp>

class ModelData;
typedef boost::shared_ptr<ModelData> ModelDataPtr;

class ModelAccess : public boost::enable_shared_from_this<ModelAccess>, private boost::noncopyable {
public:
    virtual ~ModelAccess();
    virtual ModelDataPtr find(const SensorTime& st) = 0;

public:
    boost::signal1<void, ModelDataPtr> modelDataChanged;
};
typedef boost::shared_ptr<ModelAccess> ModelAccessPtr;

#endif // ModelAccess_hh
