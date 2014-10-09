
#include "extremes/ExtremesFilter.hh"

#include "common/KvHelpers.hh"
#include "common/KvMetaDataBuffer.hh"
#include "common/KvServiceHelper.hh"
#include "common/TimeBuffer.hh"

#define LOAD_DECL_ONLY
#include "load_17000_20141002.cc"

#define MILOGGER_CATEGORY "kvhqc.test.ExtremesTest"
#include "common/ObsLogging.hh"

static inline const Sensor& s(ObsData_p obs) { return obs->sensorTime().sensor; }
static inline const std::string t(ObsData_p obs) { return timeutil::to_iso_extended_string(obs->sensorTime().time); }
static inline float cv(ObsData_p obs) { return obs->corrected(); }

TEST(ExtremesTest, Filter)
{
  FakeKvApp fa(false); // no threading
  KvServiceHelper kvsh;
  KvMetaDataBuffer kvmdbuf;
  kvmdbuf.setHandler(fa.obsAccess()->handler());

  load_17000_20141002(fa);
  KvMetaDataBuffer::instance()->reload();

  ExtremesFilter_p ef(new ExtremesFilter(kvalobs::PARAMID_TAX, 5));
  TimeBuffer_p b = boost::make_shared<TimeBuffer>(Sensor_s(), t_17000_20141002(), ef);
  b->syncRequest(FakeKvApp::app()->obsAccess());

  // grep '\<21[15]\>' src/extremes/test/data_17000_20141002.txt | sort -rn -k 7 | head -n 20
  // omit 216 in grep as it is aggregated / not in obs_pgm

  ASSERT_EQ(7, b->size()); // two have TA=TAX

  ObsData_pv d(b->data().begin(), b->data().end());
  std::sort(d.begin(), d.end(), ObsData_by_Corrected(not ef->isMaximumSearch()));

  const eq_Sensor eq;

  EXPECT_TRUE(eq(Sensor(17380, 215, 0, 0, 502), s(d[0])));
  EXPECT_TRUE(eq(Sensor(17050, 215, 0, 0, 502), s(d[1])));
  EXPECT_TRUE(eq(Sensor(17050, 215, 0, 0, 502), s(d[2])));
  EXPECT_TRUE(eq(Sensor(17150, 215, 0, 0, 342), s(d[3])));
  EXPECT_TRUE(eq(Sensor(17000, 211, 0, 0, 330), s(d[4])));
  EXPECT_TRUE(eq(Sensor(17000, 215, 0, 0, 330), s(d[5])));
  EXPECT_TRUE(eq(Sensor(18420, 215, 0, 0, 514), s(d[6])));

  EXPECT_EQ("2014-10-01 12:00:00", t(d[0]));
  EXPECT_EQ("2014-10-01 11:00:00", t(d[1]));
  EXPECT_EQ("2014-10-01 12:00:00", t(d[2]));
  EXPECT_EQ("2014-10-01 12:00:00", t(d[3]));
  EXPECT_EQ("2014-10-02 05:00:00", t(d[4]));
  EXPECT_EQ("2014-10-02 05:00:00", t(d[5]));
  EXPECT_EQ("2014-10-01 11:00:00", t(d[6]));

  EXPECT_FLOAT_EQ(16.8, cv(d[0]));
  EXPECT_FLOAT_EQ(15.7, cv(d[1]));
  EXPECT_FLOAT_EQ(15.7, cv(d[2]));
  EXPECT_FLOAT_EQ(15.4, cv(d[3]));
  EXPECT_FLOAT_EQ(14.7, cv(d[4]));
  EXPECT_FLOAT_EQ(14.7, cv(d[5]));
  EXPECT_FLOAT_EQ(14.6, cv(d[6]));

  // (s:17380, p:215, l:0, s:0, t:502)@2014-10-01 12:00:00 c= ci=0111100000000010
  // (s:17050, p:215, l:0, s:0, t:502)@2014-10-01 11:00:00 c= ci=0111100000000010
  // (s:17050, p:215, l:0, s:0, t:502)@2014-10-01 12:00:00 c= ci=0111100000000010
  // (s:17150, p:215, l:0, s:0, t:342)@2014-10-01 12:00:00 c= ci=0111100000000010
  // (s:17000, p:211, l:0, s:0, t:330)@2014-10-02 05:00:00 c= ci=0111100000100010
  // (s:17000, p:215, l:0, s:0, t:330)@2014-10-02 05:00:00 c= ci=0111100000000010
  // (s:18420, p:215, l:0, s:0, t:514)@2014-10-01 11:00:00 c= ci=0111100000000010
  // for (ObsData_pv::const_iterator it = d.begin(); it != d.end(); ++it)
  //   EXPECT_FALSE(*it) << (*it)->sensorTime() << " c=" << (*it)->corrected() << " ci=" << (*it)->controlinfo().flagstring();
}
