
#include "AnalyseFCC.hh"
#include "KvalobsAccess.hh"

#define LOAD_DECL_ONLY
#include "load_31850_20121130.cc"
#include "load_44160_20121207.cc"
#include "TestHelpers.hh"

TEST(AnalyseFCCTest, Basic)
{
    FakeKvApp fa;
    const Sensor sensor(44160, 110, 0, 0, 302);
    const TimeRange time(t_44160_20121207());
    load_44160_20121207(fa);

    EditAccessPtr eda = boost::make_shared<EditAccess>(fa.kda);
    eda->newVersion();
    FCC::analyse(eda, sensor, time);

    const timeutil::ptime tBad1 = s2t("2012-11-30 06:00:00"), tBad2 = s2t("2012-12-01 06:00:00");
    for (timeutil::ptime t = time.t0(); t < time.t1(); t += boost::gregorian::days(1)) {
        EditDataPtr obs = eda->findE(SensorTime(sensor, t));
        ASSERT_TRUE(obs != 0) << "t=" << t;
        ASSERT_EQ((t==tBad1 or t==tBad2), obs->hasTasks())  << "t=" << t;
    }
}

namespace FCC {
namespace /*FCC::*/detail {
extern const int N_COLUMNS;
extern const int pars[];
extern const int timeOffsets[];
} // namespace FCC::detail
} // namespace FCC

TEST(AnalyseFCCTest, SameObs)
{
    FakeKvApp fa;
    const Sensor sensor(31850, 110, 0, 0, 302);
    const TimeRange time(s2t("2012-11-15 06:00:00"), s2t("2012-11-30 06:00:00"));
    load_31850_20121130(fa);

    EditAccessPtr eda = boost::make_shared<EditAccess>(fa.kda);
    eda->newVersion();
    FCC::analyse(eda, sensor, time);

    const timeutil::ptime tBad = s2t("2012-11-21 06:00:00");
    
    for(int i=0; i<FCC::detail::N_COLUMNS; ++i) {
        Sensor ps(sensor);
        ps.paramId = FCC::detail::pars[i];
        const boost::posix_time::time_duration dt = boost::posix_time::hours(FCC::detail::timeOffsets[i]);
        for (timeutil::ptime t = time.t0(); t < time.t1(); t += boost::gregorian::days(1)) {
            EditDataPtr obs = eda->findE(SensorTime(ps, t + dt));
            if (obs) {
                ASSERT_EQ(t==tBad, obs->hasTasks())  << "t=" << t << " / p=" << ps.paramId;
            }
        }
    }
}
