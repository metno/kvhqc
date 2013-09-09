
#include "FakeKvApp.hh"
#include "EditAccess.hh"

#define LOAD_DECL_ONLY
#include "load_18210_20130410.cc"
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

TEST(KvalobsAccessTest, Subscribe)
{
    const Sensor sensor(18210, 211, 0, 0, 502);
    const timeutil::ptime t0(s2t("2013-04-04 04:00:00"));
    const TimeRange tr(s2t("2013-04-04 00:00:00"), s2t("2013-04-04 12:00:00"));
    const ObsSubscription s0(sensor.stationId, TimeRange(t0, t0));

    FakeKvApp fa;
    load_18210_20130410(fa, false);

    {
      fa.kda->addSubscription(s0);
      ObsDataPtr obs = fa.kda->find(SensorTime(sensor, t0));
      ASSERT_TRUE(obs);
      ASSERT_FLOAT_EQ(-0.7, obs->corrected());
      fa.kda->removeSubscription(s0);
    }

    EditAccessPtr eda(new EditAccess(fa.kda));
    {
      eda->addSubscription(s0);
      EXPECT_TRUE(eda->find(SensorTime(sensor, t0)));

      const ObsSubscription s1(sensor.stationId, tr);
      eda->addSubscription(s1);
      EXPECT_TRUE(eda->find(SensorTime(sensor, s2t("2013-04-04 02:00:00"))));
      eda->removeSubscription(s1);
      eda->removeSubscription(s0);
    }
}

