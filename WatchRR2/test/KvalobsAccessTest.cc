
#include "FakeKvApp.hh"

#define LOAD_DECL_ONLY
#include "load_31850_20121130.cc"
#include "load_54420_20121130.cc"

TEST(KvalobsAccessTest, Basic)
{
    const Sensor sensor(54420, 110, 0, 0, 302);
    const TimeRange time(s2t("2012-10-01 06:00:00"), s2t("2012-11-20 06:00:00"));
    FakeKvApp fa;
    load_54420_20121130(fa);

    ObsDataPtr obs = fa.kda->find(SensorTime(sensor, s2t("2012-11-06 06:00:00")));
    ASSERT_TRUE(obs);
    ASSERT_FLOAT_EQ(0.6, obs->corrected());

    obs = fa.kda->find(SensorTime(sensor, s2t("2012-11-22 06:00:00")));
    ASSERT_TRUE(obs);
    ASSERT_FLOAT_EQ(15.5, obs->original());
}

TEST(KvalobsAccessTest, AllTimes)
{
    FakeKvApp fa;
    const TimeRange time(s2t("2012-11-15 06:00:00"), s2t("2012-11-30 06:00:00"));
    load_31850_20121130(fa);

    EXPECT_EQ(16, fa.kda->allTimes(Sensor(31850, 110, 0, 0, 302), time).size());
    EXPECT_EQ(25, fa.kda->allTimes(Sensor(31850,  34, 0, 0, 302), time).size());
}
