
#ifndef FakeModelAccess_hh
#define FakeModelAccess_hh 1

#include "ModelAccess.hh"
#include "Sensor.hh"

#include <gtest/gtest.h>
#include <string>

class FakeModelAccess : public ModelAccess {
public:

    int insertStation;
    int insertParam;

    void insert(int stationId, int paramId, const std::string& obstime, float value);

    void insert(const std::string& obstime, float value)
        { insert(insertStation, insertParam, obstime, value); }

    bool erase(ModelData_p mdl);
};

HQC_TYPEDEF_P(FakeModelAccess);

#endif // FakeModelAccess_hh
