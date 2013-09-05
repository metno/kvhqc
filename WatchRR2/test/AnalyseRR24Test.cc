
#include "AnalyseRR24.hh"
#include "KvalobsAccess.hh"

#include "Tasks.hh"

#define LOAD_DECL_ONLY
#include "load_54420_20121130.cc"
#include "load_32780_20121207.cc"

#include <kvalobs/kvDataOperations.h>
#include <boost/range.hpp>

class AnalyseRR24Test : public ::testing::Test {
public:
    AnalyseRR24Test();
    void SetUp();

    FakeKvApp fa;
    const Sensor sensor;
    const TimeRange time;
};

// ========================================================================

AnalyseRR24Test::AnalyseRR24Test()
 : sensor(54420, 110, 0, 0, 302)
 , time(s2t("2012-10-01 06:00:00"), s2t("2012-11-20 06:00:00"))
{
}

void AnalyseRR24Test::SetUp()
{
    load_54420_20121130(fa);
}

// ========================================================================

TEST_F(AnalyseRR24Test, TaskPeriods)
{
    TimeRange editableTime(time);

    EditAccessPtr eda = boost::make_shared<EditAccess>(fa.kda);
    RR24::analyse(eda, sensor, editableTime);

    ASSERT_EQ("2012-10-05 06:00:00", timeutil::to_iso_extended_string(editableTime.t0()));
    ASSERT_EQ("2012-11-15 06:00:00", timeutil::to_iso_extended_string(editableTime.t1()));

    const char* times[][2] = {
        { "2012-10-05 06:00:00", "2012-10-25 06:00:00" },
        { "2012-10-30 06:00:00", "2012-10-30 06:00:00" },
        { "2012-11-02 06:00:00", "2012-11-10 06:00:00" },
        { "2012-11-13 06:00:00", "2012-11-13 06:00:00" },
        { "2012-11-25 06:00:00", "2012-11-25 06:00:00" },
        { 0, 0 }
    };
    int iTasks = 0;
    TimeRange tasks(s2t(times[iTasks][0]), s2t(times[iTasks][1]));

    using boost::gregorian::days;
    for (timeutil::ptime t = editableTime.t0()+days(1); t < editableTime.t1(); t += days(1)) {
        if (t > tasks.t1() and times[iTasks+1][0]) {
            iTasks += 1;
            tasks = TimeRange(s2t(times[iTasks][0]), s2t(times[iTasks][1]));
        }
        EditDataPtr obs = eda->findE(SensorTime(sensor, t));
        ASSERT_TRUE(obs);
        ASSERT_EQ(tasks.contains(t), obs->hasTasks()) << "t=" << t;;
    }
}

TEST_F(AnalyseRR24Test, Gap)
{
    const timeutil::ptime td0 = s2t("2012-10-16 06:00:00");
    ObsDataPtr obs = fa.kda->find(SensorTime(sensor, td0));
    ASSERT_TRUE(obs);
    ASSERT_TRUE(fa.eraseData(obs->sensorTime()));

    TimeRange editableTime(time);
    EditAccessPtr eda = boost::make_shared<EditAccess>(fa.kda);
    RR24::analyse(eda, sensor, editableTime);

    EditDataPtr ebs = eda->findE(SensorTime(sensor, td0));
    ASSERT_TRUE(obs);
    ASSERT_TRUE(ebs->hasTasks());
}

TEST_F(AnalyseRR24Test, Redistribute)
{
    const char* times[][2] = {
        { "2012-10-05 06:00:00", "2012-10-11 06:00:00" },
        { "2012-10-12 06:00:00", "2012-10-18 06:00:00" },
        { "2012-10-19 06:00:00", "2012-10-25 06:00:00" },
        { "2012-11-02 06:00:00", "2012-11-08 06:00:00" },
        { "2012-11-09 06:00:00", "2012-11-15 06:00:00" },
        { 0, 0 }
    };

    const float value = 123.4;
    {
        TimeRange editableTime(time);
        EditAccessPtr eda = boost::make_shared<EditAccess>(fa.kda);
        RR24::analyse(eda, sensor, editableTime);

        for(int i=0; times[i][0]; ++i) {
            const TimeRange timeAcc(s2t(times[i][0]), s2t(times[i][1]));
            const std::vector<float> newCorrected(timeAcc.days()+1, value);
            EditAccessPtr eda2 = boost::make_shared<EditAccess>(eda);
            RR24::redistribute(eda2, sensor, timeAcc.t0(), editableTime, newCorrected);
            eda2->sendChangesToParent();
        }
        EditDataPtr obs = eda->findE(SensorTime(sensor, s2t("2012-10-30 06:00:00")));
        ASSERT_TRUE(obs);
        eda->editor(obs)->clearTask(tasks::TASK_HQC_AUTOMATIC);
        eda->sendChangesToParent();
    }

    for(int i=0; times[i][0]; ++i) {
        const TimeRange timeAcc(s2t(times[i][0]), s2t(times[i][1]));
        for(timeutil::ptime t=timeAcc.t0(); t<=timeAcc.t1(); t += boost::gregorian::days(1)) {
            ObsDataPtr obs = fa.kda->find(SensorTime(sensor, t));
            ASSERT_TRUE(obs);
            ASSERT_NEAR(value, obs->corrected(), 0.01) << "t=" << t;
        }
    }
}

TEST_F(AnalyseRR24Test, RedistributePartialEnd)
{
    const TimeRange timeR(s2t("2012-10-22 06:00:00"), s2t("2012-10-25 06:00:00"));
    const float value = 4;
    EditAccessPtr eda = boost::make_shared<EditAccess>(fa.kda);
    const std::vector<float> newCorrected(timeR.days()+1, value);
    {
        EditAccessPtr edaR = boost::make_shared<EditAccess>(eda);
        RR24::redistribute(edaR, sensor, timeR.t0(), time, newCorrected);
        edaR->sendChangesToParent();
    }

    const TimeRange timePA(s2t("2012-10-19 06:00:00"), s2t("2012-10-21 06:00:00"));
    for (timeutil::ptime t=timePA.t0(); t<=timePA.t1(); t+=boost::gregorian::days(1)) {
        EditDataPtr obs = eda->findE(SensorTime(sensor, t));
        ASSERT_TRUE(obs);
        ASSERT_TRUE(obs->hasTask(tasks::TASK_PREVIOUSLY_ACCUMULATION)) << "t=" << t;
    }
    for (timeutil::ptime t=timeR.t0(); t<=timeR.t1(); t+=boost::gregorian::days(1)) {
        EditDataPtr obs = eda->findE(SensorTime(sensor, t));
        ASSERT_TRUE(obs);
        ASSERT_FALSE(obs->hasTasks()) << "t=" << t;
        ASSERT_EQ(value, obs->corrected()) << "t=" << t;
    }
}


TEST_F(AnalyseRR24Test, RedistributePartialMid)
{
    const TimeRange timeR(s2t("2012-10-21 06:00:00"), s2t("2012-10-24 06:00:00"));
    const float value = 3;
    EditAccessPtr eda = boost::make_shared<EditAccess>(fa.kda);
    const std::vector<float> newCorrected(timeR.days()+1, value);
    {
        EditAccessPtr edaR = boost::make_shared<EditAccess>(eda);
        RR24::redistribute(edaR, sensor, timeR.t0(), time, newCorrected);
        edaR->sendChangesToParent();
    }

    const char* times[] = {
        "2012-10-19 06:00:00", "2012-10-20 06:00:00", "2012-10-25 06:00:00", 0
    };
    for(int i=0; times[i]; ++i) {
        const timeutil::ptime t = s2t(times[i]);
        EditDataPtr obs = eda->findE(SensorTime(sensor, t));
        ASSERT_TRUE(obs);
        ASSERT_TRUE(obs->hasTask(tasks::TASK_PREVIOUSLY_ACCUMULATION)) << "t=" << t;
    }
    for (timeutil::ptime t=timeR.t0(); t<=timeR.t1(); t+=boost::gregorian::days(1)) {
        EditDataPtr obs = eda->findE(SensorTime(sensor, t));
        ASSERT_TRUE(obs);
        ASSERT_FALSE(obs->hasTasks()) << "t=" << t;
        ASSERT_EQ(value, obs->corrected()) << "t=" << t;
    }
}

// ========================================================================

TEST(AnalyseRR24Test_2, FD3_Dectect)
{
    const Sensor sensor(32780, 110, 0, 0, 302);
    const TimeRange time(t_32780_20121207());
    FakeKvApp fa;
    load_32780_20121207(fa);

    TimeRange editableTime(time);
    fa.kda->addSubscription(ObsSubscription(sensor.stationId, time));
    EditAccessPtr eda = boost::make_shared<EditAccess>(fa.kda);
    RR24::analyse(eda, sensor, editableTime);

    const char* times[] = {
        "2012-12-03 06:00:00", "2012-12-04 06:00:00", 0
    };
    for(int i=0; times[i]; ++i) {
        const timeutil::ptime t = s2t(times[i]);
        EditDataPtr obs = eda->findE(SensorTime(sensor, t));
        ASSERT_TRUE(obs);
        ASSERT_TRUE(obs->hasTask(tasks::TASK_MAYBE_ACCUMULATED)) << "t=" << t;
    }
}

// ========================================================================

TEST(AnalyseRR24Test_2, OnlyEndpointRow)
{
    using boost::gregorian::days;

    const Sensor sensor(32780, 110, 0, 0, 302);
    const TimeRange time(s2t("2012-12-01 06:00:00"), s2t("2012-12-06 06:00:00"));
    FakeKvApp fa;
    fa.insertStation = sensor.stationId;
    fa.insertParam   = sensor.paramId;
    fa.insertType    = sensor.typeId;
    fa.insertData("2012-12-01 06:00:00",      -1.0,      -1.0, "0110000000001000", "");
    // fa.insertData("2012-12-02 06:00:00",  -32767.0,  -32767.0, "0000003000002000", "QC1...");
    // fa.insertData("2012-12-03 06:00:00",  -32767.0,  -32767.0, "0000003000002000", "QC1...");
    // fa.insertData("2012-12-04 06:00:00",  -32767.0,  -32767.0, "0000003000002000", "QC1...");
    fa.insertData("2012-12-05 06:00:00",       4.0,       4.0, "0000004000004000", "");
    fa.insertData("2012-12-06 06:00:00",      -1.0,      -1.0, "0110000000001000", "");

    const TimeRange timeR(s2t("2012-12-02 06:00:00"), s2t("2012-12-05 06:00:00"));
    const std::vector<float> newCorrected(timeR.days()+1, 4.0f);
    fa.kda->addSubscription(ObsSubscription(sensor.stationId, time));
    EditAccessPtr eda = boost::make_shared<EditAccess>(fa.kda);
    RR24::redistribute(eda, sensor, timeR.t0(), timeR, newCorrected);
    eda->sendChangesToParent();

    for (timeutil::ptime t = timeR.t0(); t < timeR.t1(); t += days(1)) {
        ObsDataPtr obs = fa.kda->find(SensorTime(sensor, t));
        ASSERT_TRUE(obs);
    }
}

TEST(AnalyseRR24Test_2, MinimalRedistribute)
{
    using boost::gregorian::days;

    const Sensor sensor(32780, 110, 0, 0, 302);
    const TimeRange time(s2t("2012-12-01 06:00:00"), s2t("2012-12-06 06:00:00"));
    FakeKvApp fa;
    fa.insertStation = sensor.stationId;
    fa.insertParam   = sensor.paramId;
    fa.insertType    = sensor.typeId;
    fa.insertData("2012-12-01 06:00:00",       2.0,       2.0, "0110000000001000", "");
    fa.insertData("2012-12-02 06:00:00",  -32767.0,  -32767.0, "0000003000002000", "QC1...");
    fa.insertData("2012-12-03 06:00:00",  -32767.0,  -32767.0, "0000003000002000", "QC1...");
    fa.insertData("2012-12-04 06:00:00",  -32767.0,  -32767.0, "0000003000002000", "QC1...");
    fa.insertData("2012-12-05 06:00:00",       4.0,       4.0, "0000004000004000", "");
    fa.insertData("2012-12-06 06:00:00",       2.0,       2.0, "0110000000001000", "");

    const TimeRange timeR(s2t("2012-12-02 06:00:00"), s2t("2012-12-05 06:00:00"));
    const std::vector<float> newCorrected(timeR.days()+1, 1.0f);
    fa.kda->addSubscription(ObsSubscription(sensor.stationId, time));
    EditAccessPtr eda = boost::make_shared<EditAccess>(fa.kda);
    RR24::redistribute(eda, sensor, timeR.t0(), timeR, newCorrected);
    eda->sendChangesToParent();

    for (timeutil::ptime t = timeR.t0(); t < timeR.t1(); t += days(1)) {
        ObsDataPtr obs = fa.kda->find(SensorTime(sensor, t));
        ASSERT_TRUE(obs);
        ASSERT_NEAR(1, obs->corrected(), 0.01) << "t=" << t;
    }
}

TEST(AnalyseRR24Test_2, RedistAndSingles)
{
    using boost::gregorian::days;

    const Sensor sensor(31850, 110, 0, 0, 302);
    const TimeRange time(s2t("2012-11-22 06:00:00"), s2t("2012-11-27 06:00:00"));
    FakeKvApp fa;
    fa.insertStation = sensor.stationId;
    fa.insertParam   = sensor.paramId;
    fa.insertType    = sensor.typeId;
    fa.insertData("2012-11-22 06:00:00",       3.7,       3.7, "0110000000004004", "watchRR");
    fa.insertData("2012-11-23 06:00:00",       1.2,       1.2, "0110000000004004", "watchRR");
    fa.insertData("2012-11-24 06:00:00",  -32767.0,       6.0, "0000001000009006", "watchRR,watchRR");
    fa.insertData("2012-11-25 06:00:00",  -32767.0,      -1.0, "0000001000009006", "watchRR,watchRR");
    fa.insertData("2012-11-26 06:00:00",       8.9,       2.9, "0110004000002006", "QC1-7-110,hqc");
    fa.insertData("2012-11-27 06:00:00",       2.8,       2.8, "0110000000001000", "");

    fa.kda->addSubscription(ObsSubscription(sensor.stationId, time));
    EditAccessPtr eda = boost::make_shared<EditAccess>(fa.kda);

    const float newCorrectedR[3] = { 5, 2, 1.9 };
    const timeutil::ptime t0R(s2t("2012-11-24 06:00:00"));
    {
        const std::vector<float> newCorrected(newCorrectedR, boost::end(newCorrectedR));
        EditAccessPtr edaR = boost::make_shared<EditAccess>(eda);
        RR24::redistribute(edaR, sensor, t0R, time, newCorrected);
        eda->pushUpdate();
        edaR->sendChangesToParent();
    }

    const boost::gregorian::date_duration step = boost::gregorian::days(1);
    timeutil::ptime t = t0R;
    ASSERT_CORR_CONTROL(newCorrectedR[0], "0000001000009006", eda->findE(SensorTime(sensor, t))) << " t=" << t;
    t += step;
    ASSERT_CORR_CONTROL(newCorrectedR[1], "0000001000009006", eda->findE(SensorTime(sensor, t))) << " t=" << t;
    t += step;
    ASSERT_CORR_CONTROL(newCorrectedR[2], "011000400000A006", eda->findE(SensorTime(sensor, t))) << " t=" << t;

    const float newCorrectedS[3] = { 3.5, 1.4, 5.5 };
    {
        const timeutil::ptime t0S(s2t("2012-11-22 06:00:00"));
        const std::vector<float> newCorrected(newCorrectedS, boost::end(newCorrectedS));
        const std::vector<int> newAccept(3, RR24::AR_ACCEPT);
        EditAccessPtr edaS = boost::make_shared<EditAccess>(eda);
        RR24::singles(edaS, sensor, t0S, time, newCorrected, newAccept);
        eda->pushUpdate();
        edaS->sendChangesToParent();
    }

    t = time.t0();
    ASSERT_CORR_CONTROL(newCorrectedS[0], "0110004000001007", eda->findE(SensorTime(sensor, t))) << " t=" << t;
    t += step;
    ASSERT_CORR_CONTROL(newCorrectedS[1], "0110004000001007", eda->findE(SensorTime(sensor, t))) << " t=" << t;
    t += step;
    ASSERT_CORR_CONTROL(newCorrectedS[2], "0000001000001007", eda->findE(SensorTime(sensor, t))) << " t=" << t;

    t += step;
    EditDataPtr obs = eda->findE(SensorTime(sensor, t));
    ASSERT_CORR_CONTROL(newCorrectedR[1], "0000001000009006", obs) << " t=" << t;
    ASSERT_TRUE(obs->hasTask(tasks::TASK_PREVIOUSLY_ACCUMULATION)) << " t=" << t;

    t += step;
    obs = eda->findE(SensorTime(sensor, t));
    ASSERT_CORR_CONTROL(newCorrectedR[2], "011000400000A006", obs) << " t=" << t;
    ASSERT_TRUE(obs->hasTask(tasks::TASK_PREVIOUSLY_ACCUMULATION));
}

TEST(AnalyseRR24Test_2, AccumulationAndSingles)
{
    using boost::gregorian::days;

    const Sensor sensor(31850, 110, 0, 0, 302);
    const TimeRange time(s2t("2012-11-22 06:00:00"), s2t("2012-11-27 06:00:00"));
    FakeKvApp fa;
    fa.insertStation = sensor.stationId;
    fa.insertParam   = sensor.paramId;
    fa.insertType    = sensor.typeId;
    fa.insertData("2012-11-22 06:00:00",       3.7,       3.7, "0110000000004004", "watchRR");
    fa.insertData("2012-11-23 06:00:00",       1.2,       1.2, "0110000000004004", "watchRR");
    fa.insertData("2012-11-24 06:00:00",  -32767.0,       6.0, "0000001000009006", "watchRR,watchRR");
    fa.insertData("2012-11-25 06:00:00",  -32767.0,      -1.0, "0000001000009006", "watchRR,watchRR");
    fa.insertData("2012-11-26 06:00:00",       8.9,       2.9, "0110004000002006", "QC1-7-110,hqc");
    fa.insertData("2012-11-27 06:00:00",       2.8,       2.8, "0110000000001000", "");

    fa.kda->addSubscription(ObsSubscription(sensor.stationId, time));
    EditAccessPtr eda = boost::make_shared<EditAccess>(fa.kda);

    const float newC[6] = { 3.5, 1.4, 5.5, -1, -1, 2.9 };
    const int   newA[6] = { RR24::AR_ACCEPT, RR24::AR_ACCEPT, RR24::AR_ACCEPT, RR24::AR_NONE, RR24::AR_NONE, RR24::AR_ACCEPT };
    {
        const timeutil::ptime t0S(s2t("2012-11-22 06:00:00"));
        const std::vector<float> nc(newC, boost::end(newC));
        const std::vector<int>   na(newA, boost::end(newA));
        RR24::singles(eda, sensor, time.t0(), time, nc, na);
    }

    const boost::gregorian::date_duration step = boost::gregorian::days(1);
    int i=0;
    for(timeutil::ptime t = time.t0(); t <= time.t1(); t += step, i += 1) {
        EditDataPtr obs = eda->findE(SensorTime(sensor, t));
        ASSERT_TRUE(obs);
        ASSERT_EQ(newA[i] == RR24::AR_NONE, obs->hasTask(tasks::TASK_PREVIOUSLY_ACCUMULATION)) << " t=" << t;
    }
}

TEST(AnalyseRR24Test_2, AccumulationAndSingles2)
{
    const boost::gregorian::date_duration step = boost::gregorian::days(1);

    const Sensor sensor(54420, 110, 0, 0, 302);
    const TimeRange time(s2t("2012-09-30 06:00:00"), s2t("2012-10-17 06:00:00"));
    FakeKvApp fa;
    fa.insertStation = sensor.stationId;
    fa.insertParam   = sensor.paramId;
    fa.insertType    = sensor.typeId;
    fa.insertData("2012-09-30 06:00:00",       1.0,       1.0, "0110000000001000"); // _->_
    fa.insertData("2012-10-01 06:00:00",  -32767.0,       1.0, "0000001000007000"); // N->P
    fa.insertData("2012-10-02 06:00:00",  -32767.0,       1.0, "0000001000007000"); // A->_
    fa.insertData("2012-10-03 06:00:00",  -32767.0,       1.0, "0000001000007000"); // A->_
    fa.insertData("2012-10-04 06:00:00",       4.0,       1.0, "0110004000008000"); // N->P
    fa.insertData("2012-10-05 06:00:00",       1.0,       1.0, "0110000000001000"); // A->_
    fa.insertData("2012-10-06 06:00:00",  -32767.0,       1.0, "0000001000007000"); // A->_
    fa.insertData("2012-10-07 06:00:00",  -32767.0,       1.0, "0000001000007000"); // A->_
    const bool fd7_error = false;
    fa.insertData("2012-10-08 06:00:00",       3.0,       1.0, (fd7_error)
                ? "0110004000007000"   // N->P // fd=7 is a data error!
                : "0110004000008000"); // N->P
    fa.insertData("2012-10-09 06:00:00",  -32767.0,       1.0, "0000001000007000"); // N->_/P (p if fd7_error)
    fa.insertData("2012-10-10 06:00:00",       2.0,       1.0, "0110004000008000"); // N->_/P (p if fd7_error)
    fa.insertData("2012-10-11 06:00:00",       1.0,       1.0, "0110000000001000"); // R->_
    fa.insertData("2012-10-12 06:00:00",  -32767.0,       1.0, "0000001000007000"); // A->_
    fa.insertData("2012-10-13 06:00:00",       2.0,       1.0, "0110004000008000"); // N->P
    fa.insertData("2012-10-14 06:00:00",  -32767.0,       1.0, "0000001000007000"); // N->_
    fa.insertData("2012-10-15 06:00:00",       1.0,       1.0, "0110004000008000"); // N->_
    fa.insertData("2012-10-16 06:00:00",  -32767.0,       1.0, "0000001000007000"); // A->_
    fa.insertData("2012-10-17 06:00:00",       1.0,       1.0, "0110004000008000"); // _->P

    fa.kda->addSubscription(ObsSubscription(sensor.stationId, time));
    EditAccessPtr eda = boost::make_shared<EditAccess>(fa.kda);

    const int NN = 16, A = RR24::AR_ACCEPT, N = RR24::AR_NONE, R = RR24::AR_REJECT;
    const int   newA[NN] = { /*10-01*/ N, A, A, N, A,
                             /*10-06*/ A, A, N, N, N,
                             /*10-11*/ R, A, N, N, N,
                             /*10-16*/ A };
    {
        const timeutil::ptime t0S(s2t("2012-10-01 06:00:00"));
        const std::vector<float> nc(NN, 2.0);
        const std::vector<int>   na(newA, boost::end(newA));
        RR24::singles(eda, sensor, time.t0()+step, time, nc, na);
    }

    const int markedPi[5] = {1,4,8,13,17};
    std::set<int> markedP(markedPi, boost::end(markedPi));
    if (fd7_error) {
        markedP.insert(9);
        markedP.insert(10);
    }
    timeutil::ptime t = time.t0();
    for(int i=0; i < NN; t += step, i += 1) {
        EditDataPtr obs = eda->findE(SensorTime(sensor, t));
        ASSERT_TRUE(obs);
        bool shouldHavePreviousTask = (markedP.find(i) != markedP.end());
        ASSERT_EQ(shouldHavePreviousTask, obs->hasTask(tasks::TASK_PREVIOUSLY_ACCUMULATION)) << " t=" << t;
    }
}

TEST(AnalyseRR24Test_2, SameCorrectedAsOrig)
{
    const Sensor sensor(84070, 110, 0, 0, 302);
    const TimeRange time(s2t("2012-09-20 06:00:00"), s2t("2012-09-25 06:00:00"));
    FakeKvApp fa;
    fa.insertStation = sensor.stationId;
    fa.insertParam   = sensor.paramId;
    fa.insertType    = sensor.typeId;
    fa.insertData("2012-09-20 06:00:00",      -1.0,      -1.0, "0110000000000000", "");
    fa.insertData("2012-09-21 06:00:00",      -1.0,      17.0, "000000400000A006", "QC...");
    fa.insertData("2012-09-22 06:00:00",  -32767.0,       0.9, "0000001000007006", "QC...");
    fa.insertData("2012-09-23 06:00:00",      18.9,       1.0, "000000400000A006", "QC...");
    fa.insertData("2012-09-24 06:00:00",      13.0,      11.1, "0110004000007006", "QC...");
    fa.insertData("2012-09-25 06:00:00",       0.2,       0.2, "0110000000001000", "");

    fa.kda->addSubscription(ObsSubscription(sensor.stationId, time));
    EditAccessPtr eda = boost::make_shared<EditAccess>(fa.kda);

    const timeutil::ptime t0S(s2t("2012-09-24 06:00:00"));
    const std::vector<float> nc(1, 13.0);
    const std::vector<int>   na(1, RR24::AR_ACCEPT);
    RR24::singles(eda, sensor, t0S, time, nc, na);

    EditDataPtr obs = eda->findE(SensorTime(sensor, t0S));
    ASSERT_TRUE(obs);
    ASSERT_EQ(4, obs->controlinfo().flag((kvalobs::flag::fmis)));
}

TEST(AnalyseRR24Test_2, RedistEndDryAsBefore)
{
    const Sensor sensor(84070, 110, 0, 0, 302);
    const TimeRange time(s2t("2012-09-22 06:00:00"), s2t("2012-09-23 06:00:00"));
    FakeKvApp fa;
    fa.insertStation = sensor.stationId;
    fa.insertParam   = sensor.paramId;
    fa.insertType    = sensor.typeId;
    fa.insertData("2012-09-22 06:00:00",    -32767,    -32767, "0000003000002000", "QC...");
    fa.insertData("2012-09-23 06:00:00",      -1.0,      -1.0, "0000004000004000", "QC...");

    fa.kda->addSubscription(ObsSubscription(sensor.stationId, time));
    EditAccessPtr eda = boost::make_shared<EditAccess>(fa.kda);

    std::vector<float> nc(2);
    nc[0] = -1;
    nc[1] = -1;
    RR24::redistribute(eda, sensor, time.t0(), time, nc);

    EditDataPtr obs = eda->findE(SensorTime(sensor, time.t0()));
    ASSERT_TRUE(obs);
    EXPECT_EQ(1, obs->controlinfo().flag((kvalobs::flag::fmis)));
    EXPECT_EQ(9, obs->controlinfo().flag((kvalobs::flag::fd)));
    EXPECT_EQ(6, obs->controlinfo().flag((kvalobs::flag::fhqc)));

    obs = eda->findE(SensorTime(sensor, time.t1()));
    ASSERT_TRUE(obs);
    // next fmis is important: as both old and new corrected are -1, is has to be 0
    EXPECT_EQ( 0, obs->controlinfo().flag((kvalobs::flag::fmis)));
    EXPECT_EQ(10, obs->controlinfo().flag((kvalobs::flag::fd)));
    EXPECT_EQ( 6, obs->controlinfo().flag((kvalobs::flag::fhqc)));
}

TEST(AnalyseRR24Test_2, RedistEndDryNew)
{
    const Sensor sensor(84070, 110, 0, 0, 302);
    const TimeRange time(s2t("2012-09-22 06:00:00"), s2t("2012-09-23 06:00:00"));
    FakeKvApp fa;
    fa.insertStation = sensor.stationId;
    fa.insertParam   = sensor.paramId;
    fa.insertType    = sensor.typeId;
    fa.insertData("2012-09-22 06:00:00",    -32767,    -32767, "0000003000002000", "QC...");
    fa.insertData("2012-09-23 06:00:00",       0.0,       0.0, "0000004000004000", "QC...");

    fa.kda->addSubscription(ObsSubscription(sensor.stationId, time));
    EditAccessPtr eda = boost::make_shared<EditAccess>(fa.kda);

    std::vector<float> nc(2);
    nc[0] =  0;
    nc[1] = -1;
    RR24::redistribute(eda, sensor, time.t0(), time, nc);

    EditDataPtr obs = eda->findE(SensorTime(sensor, time.t0()));
    ASSERT_TRUE(obs);
    EXPECT_EQ(1, obs->controlinfo().flag((kvalobs::flag::fmis)));
    EXPECT_EQ(9, obs->controlinfo().flag((kvalobs::flag::fd)));
    EXPECT_EQ(6, obs->controlinfo().flag((kvalobs::flag::fhqc)));

    obs = eda->findE(SensorTime(sensor, time.t1()));
    ASSERT_TRUE(obs);
    EXPECT_EQ( 4, obs->controlinfo().flag((kvalobs::flag::fmis)));
    EXPECT_EQ(10, obs->controlinfo().flag((kvalobs::flag::fd)));
    EXPECT_EQ( 6, obs->controlinfo().flag((kvalobs::flag::fhqc)));
}

TEST(AnalyseRR24Test_2, QC2_1)
{
    const Sensor sensor(31850, 110, 0, 0, 302);
    FakeKvApp fa;
    fa.insertStation = sensor.stationId;
    fa.insertParam   = sensor.paramId;
    fa.insertType    = sensor.typeId;
    fa.insertData("2012-11-23 06:00:00",       1.2,       1.2, "0110000000004004", "watchRR");
    fa.insertData("2012-11-24 06:00:00",  -32767.0,       6.0, "0000001000009006", "watchRR,watchRR");
    fa.insertData("2012-11-25 06:00:00",  -32767.0,      -1.0, "0000001000009006", "watchRR,watchRR");
    fa.insertData("2012-11-26 06:00:00",       8.9,       2.9, "0110004000002006", "QC1-7-110,hqc");
    fa.insertData("2012-11-27 06:00:00",       2.8,       2.8, "0110000000001000", "");

    const TimeRange time(s2t("2012-11-24 06:00:00"), s2t("2012-11-26 06:00:00"));
    fa.kda->addSubscription(ObsSubscription(sensor.stationId, time));
    EditAccessPtr eda = boost::make_shared<EditAccess>(fa.kda);

    ASSERT_FALSE(RR24::canRedistributeInQC2(eda, sensor, time));
}

TEST(AnalyseRR24Test_2, Accept)
{
    const boost::gregorian::date_duration step = boost::gregorian::days(1);

    const Sensor sensor(31850, 110, 0, 0, 302);
    FakeKvApp fa;
    fa.insertStation = sensor.stationId;
    fa.insertParam   = sensor.paramId;
    fa.insertType    = sensor.typeId;
    fa.insertData("2012-11-23 06:00:00",       1.2,       1.2, "0110000000004004", "watchRR");
    fa.insertData("2012-11-24 06:00:00",  -32767.0,       6.0, "0000001000009006", "watchRR,watchRR");
    fa.insertData("2012-11-25 06:00:00",  -32767.0,      -1.0, "0000001000009006", "watchRR,watchRR");
    fa.insertData("2012-11-26 06:00:00",       8.9,       2.9, "0110004000002006", "QC1-7-110,hqc");
    fa.insertData("2012-11-27 06:00:00",  -32767.0,       6.0, "0000001000007000", "QC2-redist");
    fa.insertData("2012-11-28 06:00:00",  -32767.0,      -1.0, "0000001000007000", "QC2-redist");
    fa.insertData("2012-11-29 06:00:00",       8.9,       2.9, "0110004000002000", "QC1-7-110");
    fa.insertData("2012-11-30 06:00:00",  -32767.0,       6.0, "0000001000007000", "QC2-redist");
    fa.insertData("2012-12-01 06:00:00",  -32767.0,      -1.0, "0000001000007000", "QC2-redist");
    fa.insertData("2012-12-02 06:00:00",       8.9,       2.9, "0110004000008000", "QC1-7-110,QC2-redist");
    fa.insertData("2012-12-03 06:00:00",       2.8,       2.8, "0110000000001000", "");

    const TimeRange time_all(s2t("2012-11-23 06:00:00"), s2t("2012-12-03 06:00:00"));
    fa.kda->addSubscription(ObsSubscription(sensor.stationId, time_all));
    EditAccessPtr eda = boost::make_shared<EditAccess>(fa.kda);

    TimeRange editableTime(time_all);
    RR24::analyse(eda, sensor, editableTime);

    {
        const TimeRange time(s2t("2012-11-24 06:00:00"), s2t("2012-11-26 06:00:00"));
        EXPECT_FALSE(RR24::canAccept(eda, sensor, time));
    }

    {
        // some tasks in this time range
        const TimeRange time(s2t("2012-11-27 06:00:00"), s2t("2012-11-29 06:00:00"));
        EXPECT_FALSE(RR24::canAccept(eda, sensor, time));
    }

    {
        const TimeRange time(s2t("2012-11-30 06:00:00"), s2t("2012-12-02 06:00:00"));
        ASSERT_TRUE(RR24::canAccept(eda, sensor, time));

        RR24::accept(eda, sensor, time);
        for (timeutil::ptime t = time.t0(); t <= time.t1(); t += step) {
            EditDataPtr obs = eda->findE(SensorTime(sensor, t));
            ASSERT_TRUE(obs) << " t=" << t;
            EXPECT_EQ(1, obs->controlinfo().flag(kvalobs::flag::fhqc)) << " t=" << t;
        }
    }
}

TEST(AnalyseRR24Test_2, CalculateSum)
{
    using boost::gregorian::days;

    const Sensor sensor(32780, 110, 0, 0, 302);
    const TimeRange time(s2t("2012-12-01 06:00:00"), s2t("2012-12-06 06:00:00"));
    FakeKvApp fa;
    fa.insertStation = sensor.stationId;
    fa.insertParam   = sensor.paramId;
    fa.insertType    = sensor.typeId;
    fa.insertData("2012-12-01 06:00:00",       2.0,       2.0, "0110000000001000", "");
    fa.insertData("2012-12-02 06:00:00",  -32767.0,  -32767.0, "0000003000002000", "QC1...");
    fa.insertData("2012-12-03 06:00:00",  -32767.0,  -32767.0, "0000003000002000", "QC1...");
    fa.insertData("2012-12-04 06:00:00",  -32767.0,  -32767.0, "0000003000002000", "QC1...");
    fa.insertData("2012-12-05 06:00:00",       -1.0,     -1.0, "0000004000004000", "");
    fa.insertData("2012-12-06 06:00:00",       2.0,       2.0, "0110000000001000", "");

    fa.kda->addSubscription(ObsSubscription(sensor.stationId, time));
    EditAccessPtr eda = boost::make_shared<EditAccess>(fa.kda);

    const TimeRange timeS1(s2t("2012-12-02 06:00:00"), s2t("2012-12-05 06:00:00"));
    ASSERT_EQ(0, RR24::calculateSum(eda, sensor, timeS1));

    const TimeRange timeS2(s2t("2012-12-05 06:00:00"), s2t("2012-12-06 06:00:00"));
    ASSERT_EQ(kvalobs::MISSING, RR24::calculateOriginalSum(eda, sensor, timeS2));
}
