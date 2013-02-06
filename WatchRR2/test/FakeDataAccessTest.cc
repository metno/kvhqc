
#include "AnalyseFCC.hh"
#include "AnalyseRR24.hh"
#include "Helpers.hh"
#include <boost/bind.hpp>

#define LOAD_DECL_ONLY
#include "load_54420_20121130.cc"
#include "load_44160_20121207.cc"

#define NDEBUG
#include "w2debug.hh"

TEST(FakeDataAccessTest, Basic)
{
    const Sensor sensor(54420, 110, 0, 0, 302);
    const TimeRange time(s2t("2012-10-01 06:00:00"), s2t("2012-11-20 06:00:00"));
    FakeDataAccessPtr fda = boost::make_shared<FakeDataAccess>();
    load_54420_20121130(fda);

    ObsDataPtr obs = fda->find(SensorTime(sensor, s2t("2012-11-06 06:00:00")));
    ASSERT_FALSE(not obs);
    ASSERT_FLOAT_EQ(0.6, obs->corrected());

    obs = fda->find(SensorTime(sensor, s2t("2012-11-22 06:00:00")));
    ASSERT_FALSE(not obs);
    ASSERT_FLOAT_EQ(15.5, obs->original());
}

// ========================================================================

struct CountDataChanged : private boost::noncopyable
{
    int count;
    ObsAccess::ObsDataChange filterWhat;
    CountDataChanged() : count(0), filterWhat(ObsAccess::MODIFIED) { }
    void operator()(ObsAccess::ObsDataChange what, ObsDataPtr obs)
        { if (what == filterWhat and obs->sensorTime().sensor.paramId == kvalobs::PARAMID_RR_24) { count += 1; } }
};

TEST(FakeDataAccessTest, DataChangedLoad54420)
{
    const Sensor sensor(54420, 110, 0, 0, 302);
    const TimeRange time(s2t("2012-10-01 06:00:00"), s2t("2012-11-20 06:00:00"));
    FakeDataAccessPtr fda = boost::make_shared<FakeDataAccess>();

    CountDataChanged cdc;
    cdc.filterWhat = ObsAccess::CREATED;
    fda->obsDataChanged.connect(boost::ref(cdc));

    load_54420_20121130(fda);
    ASSERT_EQ(72, cdc.count);
}

TEST(FakeDataAccessTest, DataChangedModifyMerge)
{
    const Sensor sensor(44160, 110, 0, 0, 302);
    const TimeRange time(t_44160_20121207());
    FakeDataAccessPtr fda = boost::make_shared<FakeDataAccess>();

    load_44160_20121207(fda);
    EditAccessPtr eda = boost::make_shared<EditAccess>(fda);

    CountDataChanged cdcF, cdcE;
    fda->obsDataChanged.connect(boost::ref(cdcF));
    eda->obsDataChanged.connect(boost::ref(cdcE));

    FCC::analyse(eda, sensor, time);

    ASSERT_EQ(0, cdcF.count);
    ASSERT_EQ(2, cdcE.count);

    eda->sendChangesToParent();

    ASSERT_EQ(0, cdcF.count); // analyse only sets tasks, which are not kept in fda
    ASSERT_EQ(2, cdcE.count);
}

TEST(FakeDataAccessTest, PopChange)
{
    const Sensor sensor(54420, 110, 0, 0, 302);
    const TimeRange time(s2t("2012-10-01 06:00:00"), s2t("2012-11-20 06:00:00"));
    FakeDataAccessPtr fda = boost::make_shared<FakeDataAccess>();
    load_54420_20121130(fda);

    EditAccessPtr eda = boost::make_shared<EditAccess>(fda);

    CountDataChanged cdcF, cdcE, cdcE2;
    fda->obsDataChanged.connect(boost::ref(cdcF));
    eda->obsDataChanged.connect(boost::ref(cdcE));

    TimeRange editableTime(time);
    RR24::analyse(eda, sensor, editableTime);

    ASSERT_EQ( 0, cdcF.count);
    ASSERT_EQ(33, cdcE.count);
    cdcE.count = 0;
    
    const TimeRange timeAcc(s2t("2012-10-05 06:00:00"), s2t("2012-10-11 06:00:00"));
    const std::vector<float> newCorrected(timeAcc.days()+1, 123.4);

    EditAccessPtr eda2 = boost::make_shared<EditAccess>(eda);
    eda2->obsDataChanged.connect(boost::ref(cdcE2));
    RR24::redistribute(eda2, sensor, timeAcc.t0(), editableTime, newCorrected);

    ASSERT_EQ(7, cdcE2.count);
    ASSERT_EQ(0, cdcE .count);

    eda->pushUpdate();
    eda2->sendChangesToParent();
    ASSERT_EQ(7, cdcE.count);
    cdcE.count = 0;

    eda->popUpdate();
    ASSERT_EQ(7, cdcE.count); // pop 7 days with changes

    eda->sendChangesToParent();
    ASSERT_EQ(0, cdcF.count); // only tasks from RR24::analyse remain, but fda does not keep tasks
}
