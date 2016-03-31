
#ifndef FakeModelAccess_hh
#define FakeModelAccess_hh 1

#include "KvModelAccess.hh"
#include "Sensor.hh"
#include "TimeRange.hh"

#include <gtest/gtest.h>
#include <memory>
#include <string>

class FakeModelAccess : public KvModelAccess {
public:

    int insertStation;
    int insertParam;

    void insert(int stationId, int paramId, const std::string& obstime, float value);

    void insert(const std::string& obstime, float value)
        { insert(insertStation, insertParam, obstime, value); }

    bool erase(ModelDataPtr mdl);
};

typedef std::shared_ptr<FakeModelAccess> FakeModelAccessPtr;

#endif // FakeModelAccess_hh
