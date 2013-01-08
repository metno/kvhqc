
#ifndef FakeDataAccess_hh
#define FakeDataAccess_hh 1

#include "KvBufferedAccess.hh"
#include "Sensor.hh"
#include "TimeRange.hh"

#include <kvalobs/kvData.h>
#include <gtest/gtest.h>
#include <boost/make_shared.hpp>
#include <string>

class FakeDataAccess : public KvBufferedAccess {
public:

    int insertStation;
    int insertParam;
    int insertType;

    void insert(int stationId, int paramId, int typeId, const std::string& obstime, float orig, float corr,
                const std::string& controlinfo="0000000000000000", const std::string& cfailed="");

    void insert(const std::string& obstime, float orig, float corr,
                const std::string& controlinfo="0000000000000000", const std::string& cfailed="")
        { insert(insertStation, insertParam, insertType, obstime, orig, corr, controlinfo, cfailed); }

    void insert(const std::string& obstime, float orig_corr,
                const std::string& controlinfo="0000000000000000", const std::string& cfailed="")
        { insert(insertStation, insertParam, insertType, obstime, orig_corr, orig_corr, controlinfo, cfailed); }
    
    bool erase(ObsDataPtr obs);

protected:
    virtual bool isSubscribed(const SensorTime& st);
};

typedef boost::shared_ptr<FakeDataAccess> FakeDataAccessPtr;

inline timeutil::ptime s2t(const std::string& t)
{ return timeutil::from_iso_extended_string(t); }

// ========================================================================

::testing::AssertionResult AssertCorrControl(const char* ec_expr, const char* eci_expr, const char* a_expr,
                                             float ec, const std::string& eci, const ObsDataPtr& a);

#define EXPECT_CORR_CONTROL(ec, eci, a) EXPECT_PRED_FORMAT3(AssertCorrControl, ec, eci, a)
#define ASSERT_CORR_CONTROL(ec, eci, a) ASSERT_PRED_FORMAT3(AssertCorrControl, ec, eci, a)

#endif // FakeDataAccess_hh
