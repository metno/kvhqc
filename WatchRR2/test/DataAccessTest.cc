
#include "FakeDataAccess.hh"

#define LOAD_DECL_ONLY
#include "load_31850_20121130.cc"

TEST(DataAccessTest, AllTimes)
{
    FakeKvApp fa;
    const TimeRange time(s2t("2012-11-15 06:00:00"), s2t("2012-11-30 06:00:00"));
    load_31850_20121130(fa);

    EXPECT_EQ(16, fa.kda->allTimes(Sensor(31850, 110, 0, 0, 302), time).size());
    EXPECT_EQ(25, fa.kda->allTimes(Sensor(31850,  34, 0, 0, 302), time).size());
}
