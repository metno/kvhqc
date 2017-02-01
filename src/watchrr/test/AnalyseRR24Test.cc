
#include "AnalyseRR24.hh"
#include "TaskAccess.hh"
#include "TaskData.hh"
#include "TaskUpdate.hh"

#include "common/KvHelpers.hh"
#include "common/Tasks.hh"
#include "common/TimeBuffer.hh"

#define LOAD_DECL_ONLY
#include "common/test/load_54420_20121130.cc"
#include "common/test/load_32780_20121207.cc"

#include <kvalobs/kvDataOperations.h>
#include <boost/range.hpp>

#include <set>

class AnalyseRR24Test : public ::testing::Test {
public:
  AnalyseRR24Test();
  void SetUp();

  FakeKvApp fa;
  const Sensor sensor;
  const TimeSpan time;
};

namespace /* anonymous */ {
int allTasks(ObsData_p obs)
{
  if (TaskData_p td = boost::dynamic_pointer_cast<TaskData>(obs))
    return td->allTasks();
  else
    return 0;
}
bool hasTasks(ObsData_p obs)
{
  if (TaskData_p td = boost::dynamic_pointer_cast<TaskData>(obs))
    return td->hasTasks();
  else
    return false;
}
bool hasTask(ObsData_p obs, int task)
{
  if (TaskData_p td = boost::dynamic_pointer_cast<TaskData>(obs))
    return td->hasTask(task);
  else
    return false;
}
Sensor_s make_set(const Sensor& s1)
{
  Sensor_s s;
  s.insert(s1);
  return s;
}
} // anonymous namespace

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
  TimeSpan editableTime(time);

  TaskAccess_p eda = boost::make_shared<TaskAccess>(fa.obsAccess());
  eda->newVersion();
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
  TimeSpan tasks(s2t(times[iTasks][0]), s2t(times[iTasks][1]));

  using boost::gregorian::days;
  for (timeutil::ptime t = editableTime.t0()+days(1); t < editableTime.t1(); t += days(1)) {
    if (t > tasks.t1() and times[iTasks+1][0]) {
      iTasks += 1;
      tasks = TimeSpan(s2t(times[iTasks][0]), s2t(times[iTasks][1]));
    }
    ObsData_p obs = eda->findE(SensorTime(sensor, t));
    ASSERT_TRUE((bool)obs);
    ASSERT_EQ(tasks.contains(t), hasTasks(obs)) << "t=" << t;;
  }
}

TEST_F(AnalyseRR24Test, Gap)
{
  const Sensor s0(45420, 110, 0, 0, 302);
  fa.insertStation = s0.stationId;
  fa.insertParam = s0.paramId;
  fa.insertType = s0.typeId;

  fa.insertData("2012-10-11 06:00:00",       4.5,       0.4, "0110004000004006", "QC1-7-110,watchRR");
  fa.insertData("2012-10-12 06:00:00",  -32767.0,       2.0, "0000001000007000", "QC1-7-110,QC2N_xx,QC2-redist,QC1-7-110");
  // fa.insertData("2012-10-13 06:00:00",  -32767.0,       2.0, "0000001000007000", "QC1-7-110,QC2N_xx,QC2-redist,QC1-7-110");
  fa.insertData("2012-10-14 06:00:00",  -32767.0,       2.0, "0000001000007000", "QC1-7-110,QC2N_xx,QC2-redist,QC1-7-110");
  fa.insertData("2012-10-15 06:00:00",      16.0,       2.0, "0110004000004006", "QC1-7-110,watchRR");
  fa.insertData("2012-10-16 06:00:00",  -32767.0,       6.9, "0000001000007000", "QC1-7-110,QC2N_xx,QC2-redist,QC2N_xx,QC2-redist,QC1-7-110");

  const TimeSpan time(s2t("2012-10-11 06:00:00"), s2t("2012-10-16 06:00:00"));
  TimeSpan editableTime(time);

  TaskAccess_p eda = boost::make_shared<TaskAccess>(fa.obsAccess());
  eda->newVersion();
  RR24::analyse(eda, s0, editableTime);

  const timeutil::ptime t0 = s2t("2012-10-13 06:00:00");
  ObsData_p obs = eda->findE(SensorTime(s0, t0));
  ASSERT_TRUE((bool)obs) << "expected to find ObsData in gap";
  ASSERT_TRUE(hasTasks(obs)) << "expected gap ObsData to have tasks";
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
    TimeSpan editableTime(time);
    TaskAccess_p eda = boost::make_shared<TaskAccess>(fa.obsAccess());
    eda->newVersion();
    RR24::analyse(eda, sensor, editableTime);

    for(int i=0; times[i][0]; ++i) {
      const TimeSpan timeAcc(s2t(times[i][0]), s2t(times[i][1]));
      const std::vector<float> newCorrected(timeAcc.days()+1, value);
      TaskAccess_p eda2 = boost::make_shared<TaskAccess>(eda);
      RR24::redistribute(eda2, sensor, timeAcc.t0(), editableTime, newCorrected);
      eda2->storeToBackend();
    }
    ObsData_p obs = eda->findE(SensorTime(sensor, s2t("2012-10-30 06:00:00")));
    ASSERT_TRUE((bool)obs);
    TaskUpdate_p tupdate = boost::static_pointer_cast<TaskUpdate>(eda->createUpdate(obs));
    tupdate->clearTask(tasks::TASK_HQC_AUTOMATIC);
    eda->storeUpdates(ObsUpdate_pv(1, tupdate));
    eda->storeToBackend();
  }

  for(int i=0; times[i][0]; ++i) {
    const TimeSpan timeAcc(s2t(times[i][0]), s2t(times[i][1]));

    TimeBuffer_p b = boost::make_shared<TimeBuffer>(make_set(sensor), timeAcc);
    b->syncRequest(fa.obsAccess());

    for(timeutil::ptime t=timeAcc.t0(); t<=timeAcc.t1(); t += boost::gregorian::days(1)) {
      ObsData_p obs = b->get(SensorTime(sensor, t));
      ASSERT_TRUE((bool)obs);
      ASSERT_NEAR(value, obs->corrected(), 0.01) << "t=" << t;
    }
  }
}

TEST_F(AnalyseRR24Test, RedistributePartialEnd)
{
  const TimeSpan timeR(s2t("2012-10-22 06:00:00"), s2t("2012-10-25 06:00:00"));
  const float value = 4;
  TaskAccess_p eda = boost::make_shared<TaskAccess>(fa.obsAccess());
  eda->newVersion();
  const std::vector<float> newCorrected(timeR.days()+1, value);
  {
    TaskAccess_p edaR = boost::make_shared<TaskAccess>(eda);
    edaR->newVersion();
    RR24::redistribute(edaR, sensor, timeR.t0(), time, newCorrected);
    edaR->storeToBackend();
  }

  const TimeSpan timePA(s2t("2012-10-19 06:00:00"), s2t("2012-10-21 06:00:00"));
  for (timeutil::ptime t=timePA.t0(); t<=timePA.t1(); t+=boost::gregorian::days(1)) {
    ObsData_p obs = eda->findE(SensorTime(sensor, t));
    ASSERT_TRUE((bool)obs);
    ASSERT_TRUE(hasTask(obs, tasks::TASK_PREVIOUSLY_ACCUMULATION)) << "t=" << t;
  }
  for (timeutil::ptime t=timeR.t0(); t<=timeR.t1(); t+=boost::gregorian::days(1)) {
    ObsData_p obs = eda->findE(SensorTime(sensor, t));
    ASSERT_TRUE((bool)obs);
    ASSERT_FALSE(hasTasks(obs)) << "t=" << t;
    ASSERT_EQ(value, obs->corrected()) << "t=" << t;
  }
}


TEST_F(AnalyseRR24Test, RedistributePartialMid)
{
  const TimeSpan timeR(s2t("2012-10-21 06:00:00"), s2t("2012-10-24 06:00:00"));
  const float value = 3;
  TaskAccess_p eda = boost::make_shared<TaskAccess>(fa.obsAccess());
  eda->newVersion();
  const std::vector<float> newCorrected(timeR.days()+1, value);
  {
    TaskAccess_p edaR = boost::make_shared<TaskAccess>(eda);
    edaR->newVersion();
    RR24::redistribute(edaR, sensor, timeR.t0(), time, newCorrected);
    edaR->storeToBackend();
  }

  const char* times[] = {
    "2012-10-19 06:00:00", "2012-10-20 06:00:00", "2012-10-25 06:00:00", 0
  };
  for(int i=0; times[i]; ++i) {
    const timeutil::ptime t = s2t(times[i]);
    ObsData_p obs = eda->findE(SensorTime(sensor, t));
    ASSERT_TRUE((bool)obs);
    ASSERT_TRUE(hasTask(obs, tasks::TASK_PREVIOUSLY_ACCUMULATION)) << "t=" << t;
  }
  for (timeutil::ptime t=timeR.t0(); t<=timeR.t1(); t+=boost::gregorian::days(1)) {
    ObsData_p obs = eda->findE(SensorTime(sensor, t));
    ASSERT_TRUE((bool)obs);
    ASSERT_FALSE(hasTasks(obs)) << "t=" << t;
    ASSERT_EQ(value, obs->corrected()) << "t=" << t;
  }
}

// ========================================================================

TEST(AnalyseRR24Test_2, FD3_Dectect)
{
  const Sensor sensor(32780, 110, 0, 0, 302);
  const TimeSpan time(t_32780_20121207());
  FakeKvApp fa;
  load_32780_20121207(fa);

  TimeSpan editableTime(time);
  TaskAccess_p eda = boost::make_shared<TaskAccess>(fa.obsAccess());
  eda->newVersion();
  RR24::analyse(eda, sensor, editableTime);

  const char* times[] = {
    "2012-12-03 06:00:00", "2012-12-04 06:00:00", 0
  };
  for(int i=0; times[i]; ++i) {
    const timeutil::ptime t = s2t(times[i]);
    ObsData_p obs = eda->findE(SensorTime(sensor, t));
    ASSERT_TRUE((bool)obs);
    ASSERT_TRUE(hasTask(obs, tasks::TASK_MAYBE_ACCUMULATED)) << "t=" << t;
  }
}

// ========================================================================

TEST(AnalyseRR24Test_2, OnlyEndpointRow)
{
  using boost::gregorian::days;

  const Sensor sensor(32780, 110, 0, 0, 302);
  const TimeSpan time(s2t("2012-12-01 06:00:00"), s2t("2012-12-06 06:00:00"));
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

  const TimeSpan timeR(s2t("2012-12-02 06:00:00"), s2t("2012-12-05 06:00:00"));
  const std::vector<float> newCorrected(timeR.days()+1, 4.0f);
  TaskAccess_p eda = boost::make_shared<TaskAccess>(fa.obsAccess());
  eda->newVersion();
  RR24::redistribute(eda, sensor, timeR.t0(), timeR, newCorrected);
  eda->storeToBackend();

  TimeBuffer_p b = boost::make_shared<TimeBuffer>(make_set(sensor), timeR);
  b->syncRequest(fa.obsAccess());

  for(timeutil::ptime t=timeR.t0(); t<=timeR.t1(); t += boost::gregorian::days(1)) {
    ObsData_p obs = b->get(SensorTime(sensor, t));
    ASSERT_TRUE((bool)obs);
  }
}

TEST(AnalyseRR24Test_2, MinimalRedistribute)
{
  using boost::gregorian::days;

  const Sensor sensor(32780, 110, 0, 0, 302);
  const TimeSpan time(s2t("2012-12-01 06:00:00"), s2t("2012-12-06 06:00:00"));
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

  const TimeSpan timeR(s2t("2012-12-02 06:00:00"), s2t("2012-12-05 06:00:00"));
  const std::vector<float> newCorrected(timeR.days()+1, 1.0f);
  TaskAccess_p eda = boost::make_shared<TaskAccess>(fa.obsAccess());
  eda->newVersion();
  RR24::redistribute(eda, sensor, timeR.t0(), timeR, newCorrected);
  eda->storeToBackend();

  TimeBuffer_p b = boost::make_shared<TimeBuffer>(make_set(sensor), timeR);
  b->syncRequest(fa.obsAccess());

  for(timeutil::ptime t=timeR.t0(); t<=timeR.t1(); t += boost::gregorian::days(1)) {
    ObsData_p obs = b->get(SensorTime(sensor, t));
    ASSERT_TRUE((bool)obs);
    ASSERT_NEAR(1, obs->corrected(), 0.01) << "t=" << t;
  }
}

TEST(AnalyseRR24Test_2, RedistAndSingles)
{
  using boost::gregorian::days;

  const Sensor sensor(31850, 110, 0, 0, 302);
  const TimeSpan time(s2t("2012-11-22 06:00:00"), s2t("2012-11-27 06:00:00"));
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

  TaskAccess_p eda = boost::make_shared<TaskAccess>(fa.obsAccess());
  eda->newVersion();

  const float newCorrectedR[3] = { 5, 2, 1.9 };
  const timeutil::ptime t0R(s2t("2012-11-24 06:00:00"));
  {
    const std::vector<float> newCorrected(newCorrectedR, boost::end(newCorrectedR));
    TaskAccess_p edaR = boost::make_shared<TaskAccess>(eda);
    edaR->newVersion();
    RR24::redistribute(edaR, sensor, t0R, time, newCorrected);
    eda->newVersion();
    edaR->storeToBackend();
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
    TaskAccess_p edaS = boost::make_shared<TaskAccess>(eda);
    edaS->newVersion();
    RR24::singles(edaS, sensor, t0S, time, newCorrected, newAccept);
    eda->newVersion();
    edaS->storeToBackend();
  }

  t = time.t0();
  ASSERT_CORR_CONTROL(newCorrectedS[0], "0110004000001007", eda->findE(SensorTime(sensor, t))) << " t=" << t;
  t += step;
  ASSERT_CORR_CONTROL(newCorrectedS[1], "0110004000001007", eda->findE(SensorTime(sensor, t))) << " t=" << t;
  t += step;
  ASSERT_CORR_CONTROL(newCorrectedS[2], "0000001000001005", eda->findE(SensorTime(sensor, t))) << " t=" << t;

  t += step;
  ObsData_p obs = eda->findE(SensorTime(sensor, t));
  ASSERT_CORR_CONTROL(newCorrectedR[1], "0000001000009006", obs) << " t=" << t;
  ASSERT_TRUE(hasTask(obs, tasks::TASK_PREVIOUSLY_ACCUMULATION)) << " t=" << t;

  t += step;
  obs = eda->findE(SensorTime(sensor, t));
  ASSERT_CORR_CONTROL(newCorrectedR[2], "011000400000A006", obs) << " t=" << t;
  ASSERT_TRUE(hasTask(obs, tasks::TASK_PREVIOUSLY_ACCUMULATION));
}

TEST(AnalyseRR24Test_2, AccumulationAndSingles)
{
  using boost::gregorian::days;

  const Sensor sensor(31850, 110, 0, 0, 302);
  const TimeSpan time(s2t("2012-11-22 06:00:00"), s2t("2012-11-27 06:00:00"));
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

  TaskAccess_p eda = boost::make_shared<TaskAccess>(fa.obsAccess());
  eda->newVersion();

  const float newC[6] = { 3.5, 1.4, 5.5, -1, -1, 2.9 };
  const int   newA[6] = { RR24::AR_ACCEPT, RR24::AR_ACCEPT, RR24::AR_ACCEPT, RR24::AR_NONE, RR24::AR_NONE, RR24::AR_ACCEPT };
  {
    const timeutil::ptime t0S(s2t("2012-11-22 06:00:00"));
    const std::vector<float> nc(newC, boost::end(newC));
    const std::vector<int>   na(newA, boost::end(newA));
    RR24::singles(eda, sensor, time.t0(), time, nc, na);
  }

  TimeBuffer_p b = boost::make_shared<TimeBuffer>(make_set(sensor), time);
  b->syncRequest(eda);
  int i=0;
  for(timeutil::ptime t=time.t0(); t<=time.t1(); t += boost::gregorian::days(1)) {
    ObsData_p obs = b->get(SensorTime(sensor, t));
    ASSERT_TRUE((bool)obs);
    ASSERT_EQ(newA[i] == RR24::AR_NONE, hasTask(obs, tasks::TASK_PREVIOUSLY_ACCUMULATION)) << " t=" << t;
  }
}

TEST(AnalyseRR24Test_2, AccumulationAndSingles2)
{
  const boost::gregorian::date_duration step = boost::gregorian::days(1);

  const Sensor sensor(54420, 110, 0, 0, 302);
  const TimeSpan time(s2t("2012-09-30 06:00:00"), s2t("2012-10-17 06:00:00"));
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

  TaskAccess_p eda = boost::make_shared<TaskAccess>(fa.obsAccess());
  eda->newVersion();

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

  TimeBuffer_p b = boost::make_shared<TimeBuffer>(make_set(sensor), time);
  b->syncRequest(eda);

  timeutil::ptime t = time.t0();
  for(int i=0; i < NN; t += step, i += 1) {
    ObsData_p obs = b->get(SensorTime(sensor, t));
    EXPECT_TRUE((bool)obs);
    const bool shouldHavePreviousTask = (markedP.find(i) != markedP.end());
    if (obs) {
      EXPECT_EQ(shouldHavePreviousTask, hasTask(obs, tasks::TASK_PREVIOUSLY_ACCUMULATION))
          << " t=" << t << " tasks=" << allTasks(obs);
    }
  }
}

TEST(AnalyseRR24Test_2, SameCorrectedAsOrig)
{
  const Sensor sensor(84070, 110, 0, 0, 302);
  const TimeSpan time(s2t("2012-09-20 06:00:00"), s2t("2012-09-25 06:00:00"));
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

  TaskAccess_p eda = boost::make_shared<TaskAccess>(fa.obsAccess());
  eda->newVersion();

  const timeutil::ptime t0S(s2t("2012-09-24 06:00:00"));
  const std::vector<float> nc(1, 13.0);
  const std::vector<int>   na(1, RR24::AR_ACCEPT);
  RR24::singles(eda, sensor, t0S, time, nc, na);

  ObsData_p obs = eda->findE(SensorTime(sensor, t0S));
  ASSERT_TRUE((bool)obs);
  ASSERT_EQ(4, obs->controlinfo().flag((kvalobs::flag::fmis)));
}

TEST(AnalyseRR24Test_2, RedistEndDryAsBefore)
{
  const Sensor sensor(84070, 110, 0, 0, 302);
  const TimeSpan time(s2t("2012-09-22 06:00:00"), s2t("2012-09-23 06:00:00"));
  FakeKvApp fa;
  fa.insertStation = sensor.stationId;
  fa.insertParam   = sensor.paramId;
  fa.insertType    = sensor.typeId;
  fa.insertData("2012-09-22 06:00:00",    -32767,    -32767, "0000003000002000", "QC...");
  fa.insertData("2012-09-23 06:00:00",      -1.0,      -1.0, "0000004000004000", "QC...");

  TaskAccess_p eda = boost::make_shared<TaskAccess>(fa.obsAccess());
  eda->newVersion();

  std::vector<float> nc(2);
  nc[0] = -1;
  nc[1] = -1;
  RR24::redistribute(eda, sensor, time.t0(), time, nc);

  ObsData_p obs = eda->findE(SensorTime(sensor, time.t0()));
  ASSERT_TRUE((bool)obs);
  EXPECT_EQ(1, obs->controlinfo().flag((kvalobs::flag::fmis)));
  EXPECT_EQ(9, obs->controlinfo().flag((kvalobs::flag::fd)));
  EXPECT_EQ(6, obs->controlinfo().flag((kvalobs::flag::fhqc)));

  obs = eda->findE(SensorTime(sensor, time.t1()));
  ASSERT_TRUE((bool)obs);
  // next fmis is important: as both old and new corrected are -1, is has to be 0
  EXPECT_EQ( 0, obs->controlinfo().flag((kvalobs::flag::fmis)));
  EXPECT_EQ(10, obs->controlinfo().flag((kvalobs::flag::fd)));
  EXPECT_EQ( 6, obs->controlinfo().flag((kvalobs::flag::fhqc)));
}

TEST(AnalyseRR24Test_2, RedistEndDryNew)
{
  const Sensor sensor(84070, 110, 0, 0, 302);
  const TimeSpan time(s2t("2012-09-22 06:00:00"), s2t("2012-09-23 06:00:00"));
  FakeKvApp fa;
  fa.insertStation = sensor.stationId;
  fa.insertParam   = sensor.paramId;
  fa.insertType    = sensor.typeId;
  fa.insertData("2012-09-22 06:00:00",    -32767,    -32767, "0000003000002000", "QC...");
  fa.insertData("2012-09-23 06:00:00",       0.0,       0.0, "0000004000004000", "QC...");

  TaskAccess_p eda = boost::make_shared<TaskAccess>(fa.obsAccess());
  eda->newVersion();

  std::vector<float> nc(2);
  nc[0] =  0;
  nc[1] = -1;
  RR24::redistribute(eda, sensor, time.t0(), time, nc);

  ObsData_p obs = eda->findE(SensorTime(sensor, time.t0()));
  ASSERT_TRUE((bool)obs);
  EXPECT_EQ(1, obs->controlinfo().flag((kvalobs::flag::fmis)));
  EXPECT_EQ(9, obs->controlinfo().flag((kvalobs::flag::fd)));
  EXPECT_EQ(6, obs->controlinfo().flag((kvalobs::flag::fhqc)));

  obs = eda->findE(SensorTime(sensor, time.t1()));
  ASSERT_TRUE((bool)obs);
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

  const TimeSpan time(s2t("2012-11-24 06:00:00"), s2t("2012-11-26 06:00:00"));
  TaskAccess_p eda = boost::make_shared<TaskAccess>(fa.obsAccess());
  eda->newVersion();

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

  const TimeSpan time_all(s2t("2012-11-23 06:00:00"), s2t("2012-12-03 06:00:00"));
  TaskAccess_p eda = boost::make_shared<TaskAccess>(fa.obsAccess());
  eda->newVersion();

  TimeSpan editableTime(time_all);
  RR24::analyse(eda, sensor, editableTime);

  {
    const TimeSpan time(s2t("2012-11-24 06:00:00"), s2t("2012-11-26 06:00:00"));
    EXPECT_FALSE(RR24::canAccept(eda, sensor, time));
  }

  {
    // some tasks in this time range
    const TimeSpan time(s2t("2012-11-27 06:00:00"), s2t("2012-11-29 06:00:00"));
    EXPECT_FALSE(RR24::canAccept(eda, sensor, time));
  }

  {
    const TimeSpan time(s2t("2012-11-30 06:00:00"), s2t("2012-12-02 06:00:00"));
    ASSERT_TRUE(RR24::canAccept(eda, sensor, time));

    RR24::accept(eda, sensor, time);
    for (timeutil::ptime t = time.t0(); t <= time.t1(); t += step) {
      ObsData_p obs = eda->findE(SensorTime(sensor, t));
      ASSERT_TRUE((bool)obs) << " t=" << t;
      EXPECT_EQ(1, obs->controlinfo().flag(kvalobs::flag::fhqc)) << " t=" << t;
    }
  }
}

TEST(AnalyseRR24Test_2, CalculateSum)
{
  using boost::gregorian::days;

  const Sensor sensor(32780, 110, 0, 0, 302);
  const TimeSpan time(s2t("2012-12-01 06:00:00"), s2t("2012-12-06 06:00:00"));
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

  TaskAccess_p eda = boost::make_shared<TaskAccess>(fa.obsAccess());

  const TimeSpan timeS1(s2t("2012-12-02 06:00:00"), s2t("2012-12-05 06:00:00"));
  ASSERT_EQ(0, RR24::calculateSum(eda, sensor, timeS1));

  const TimeSpan timeS2(s2t("2012-12-05 06:00:00"), s2t("2012-12-06 06:00:00"));
  ASSERT_EQ(kvalobs::MISSING, RR24::calculateOriginalSum(eda, sensor, timeS2));
}
