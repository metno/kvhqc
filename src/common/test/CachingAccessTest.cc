
#include "CachingAccess.hh"
#include "CountingBuffer.hh"
#include "SingleObsBuffer.hh"
#include "SqliteAccess.hh"

#include "util/make_set.hh"

#include <gtest/gtest.h>

namespace /*anonymous*/ {

inline timeutil::ptime s2t(const std::string& t)
{ return timeutil::from_iso_extended_string(t); }

} // namespace anonymous

// ========================================================================

TEST(CachingAccessTest, SingleSensor)
{
  SqliteAccess_p sqla(new SqliteAccess);
  sqla->insertDataFromFile(std::string(TEST_SOURCE_DIR)+"/data_18210_20130410.txt");
  CachingAccess_p ca(new CachingAccess(sqla));

  const Sensor sensor(18210, 211, 0, 0, 514);

  // this assumes that SqliteAccess is syncronous
  { const TimeSpan time(s2t("2013-04-01 00:00:00"), s2t("2013-04-03 00:00:00"));
    CountingBuffer_p counter(new CountingBuffer(sensor, time));
    counter->postRequest(ca);
    EXPECT_EQ(1, sqla->countPost());
    EXPECT_LE(1, counter->countNew);
    EXPECT_EQ(2*24 + 1, counter->size());
    EXPECT_EQ(1, counter->countComplete);
  }

  { const TimeSpan time(s2t("2013-04-01 00:00:00"), s2t("2013-04-02 00:00:00"));
    CountingBuffer_p counter(new CountingBuffer(sensor, time));
    counter->postRequest(ca);
    EXPECT_EQ(1, sqla->countPost());
    EXPECT_EQ(1, counter->countNew);
    EXPECT_EQ(1*24 + 1, counter->size());
    EXPECT_EQ(1, counter->countComplete);
  }

  { const TimeSpan time(s2t("2013-04-02 00:00:00"), s2t("2013-04-03 00:00:00"));
    CountingBuffer_p counter(new CountingBuffer(sensor, time));
    counter->postRequest(ca);
    EXPECT_EQ(1, sqla->countPost());
    EXPECT_EQ(1*24 + 1, counter->size());
  }

  { const TimeSpan time(s2t("2013-04-01 06:00:00"), s2t("2013-04-03 06:00:00"));
    CountingBuffer_p counter(new CountingBuffer(sensor, time));
    counter->postRequest(ca);
    EXPECT_EQ(2, sqla->countPost());
    EXPECT_EQ(2*24 + 1, counter->size());
  }

  ca->cleanCache(timeutil::now() + boost::posix_time::hours(1));

  { const TimeSpan time(s2t("2013-04-01 06:00:00"), s2t("2013-04-02 06:00:00"));
    CountingBuffer_p counter(new CountingBuffer(sensor, time));
    counter->postRequest(ca);
    EXPECT_EQ(3, sqla->countPost());
    EXPECT_EQ(1*24 + 1, counter->size());
  }
  { const TimeSpan time(s2t("2013-04-01 06:00:00"), s2t("2013-04-03 06:00:00"));
    CountingBuffer_p counter(new CountingBuffer(sensor, time));
    counter->postRequest(ca);
    EXPECT_EQ(4, sqla->countPost());
    EXPECT_EQ(2*24 + 1, counter->size());
  }
}

TEST(CachingAccessTest, MultipleSensor)
{
  SqliteAccess_p sqla(new SqliteAccess);
  sqla->insertDataFromFile(std::string(TEST_SOURCE_DIR)+"/data_18700_20140304.txt");
  CachingAccess_p ca(new CachingAccess(sqla));

  const Sensor sensor1(18210, 211, 0, 0, 514);
  const Sensor sensor2(18700, 211, 0, 0, 330);
  const Sensor_s sensors = (SetMaker<Sensor_s>() << sensor1 << sensor2).set();

  // this assumes that SqliteAccess is syncronous
  { const TimeSpan time(s2t("2014-03-01 00:00:00"), s2t("2014-03-01 06:00:00"));
    CountingBuffer_p counter(new CountingBuffer(sensor1, time));
    counter->postRequest(ca);
    EXPECT_EQ(1, sqla->countPost());
    EXPECT_EQ(1, counter->countNew);
    EXPECT_EQ(7, counter->size());
    EXPECT_EQ(1, counter->countComplete);
  }

  { const TimeSpan time(s2t("2014-03-01 03:00:00"), s2t("2014-03-01 09:00:00"));
    CountingBuffer_p counter(new CountingBuffer(sensor2, time));
    counter->postRequest(ca);
    EXPECT_EQ(2, sqla->countPost());
    EXPECT_EQ(1, counter->countNew);
    EXPECT_EQ(7, counter->size());
    EXPECT_EQ(1, counter->countComplete);
  }

  { const TimeSpan time(s2t("2014-03-01 00:00:00"), s2t("2014-03-01 09:00:00"));
    CountingBuffer_p counter(new CountingBuffer(sensors, time));
    counter->postRequest(ca);
    EXPECT_EQ(4, sqla->countPost());
    EXPECT_EQ(20, counter->size());
    EXPECT_EQ(1, counter->countComplete);
  }

  ca->cleanCache(timeutil::now() + boost::posix_time::hours(1));

  { const TimeSpan time(s2t("2014-03-01 00:00:00"), s2t("2014-03-01 09:00:00"));
    CountingBuffer_p counter(new CountingBuffer(sensors, time));
    counter->postRequest(ca);
    EXPECT_EQ(5, sqla->countPost());
    EXPECT_EQ(20, counter->size());
    EXPECT_EQ(1, counter->countComplete);
  }
}

TEST(CachingAccessTest, SingleObsAfterRange)
{
  SqliteAccess_p sqla(new SqliteAccess);
  sqla->insertDataFromFile(std::string(TEST_SOURCE_DIR)+"/data_18210_20130410.txt");
  CachingAccess_p ca(new CachingAccess(sqla));

  const Sensor sensor(18210, 211, 0, 0, 514);

  // this assumes that SqliteAccess is syncronous
  { const TimeSpan time(s2t("2013-04-01 00:00:00"), s2t("2013-04-03 00:00:00"));
    CountingBuffer_p counter(new CountingBuffer(sensor, time));
    counter->postRequest(ca);
    EXPECT_EQ(1, sqla->countPost());
    EXPECT_LE(1, counter->countNew);
    EXPECT_EQ(2*24 + 1, counter->size());
    EXPECT_EQ(1, counter->countComplete);
  }

  { const SensorTime st(sensor, s2t("2013-04-01 00:00:00"));
    SingleObsBuffer_p obs1(new SingleObsBuffer(st));
    obs1->postRequest(ca);
    EXPECT_TRUE((bool)obs1->get());
  }

  { const SensorTime st(sensor, s2t("2013-04-01 01:00:00"));
    SingleObsBuffer_p obs1(new SingleObsBuffer(st));
    obs1->postRequest(ca);
    EXPECT_TRUE((bool)obs1->get());
  }
  { const SensorTime st(sensor, s2t("2013-04-01 03:00:00"));
    SingleObsBuffer_p obs1(new SingleObsBuffer(st));
    obs1->postRequest(ca);
    EXPECT_TRUE((bool)obs1->get());
  }
}

TEST(CachingAccessTest, SingleObs)
{
  SqliteAccess_p sqla(new SqliteAccess);
  sqla->insertDataFromFile(std::string(TEST_SOURCE_DIR)+"/data_18210_20130410.txt");
  CachingAccess_p ca(new CachingAccess(sqla));

  const Sensor sensor(18210, 211, 0, 0, 514);

  // this assumes that SqliteAccess is syncronous
  { const SensorTime st(sensor, s2t("2013-04-01 00:00:00"));
    SingleObsBuffer_p obs1(new SingleObsBuffer(st));
    obs1->postRequest(ca);
    EXPECT_TRUE((bool)obs1->get());
  }
}

TEST(CachingAccessTest, Update)
{
  SqliteAccess_p sqla(new SqliteAccess);
  sqla->insertDataFromFile(std::string(TEST_SOURCE_DIR)+"/data_18210_20130410.txt");
  CachingAccess_p ca(new CachingAccess(sqla));

  const Sensor sensor(18210, 211, 0, 0, 514);

  { const TimeSpan time1(s2t("2013-04-01 00:00:00"), s2t("2013-04-01 03:00:00"));
    const TimeSpan time2(s2t("2013-04-01 00:00:00"), s2t("2013-04-01 01:00:00"));
    const TimeSpan time3(s2t("2013-04-01 02:00:00"), s2t("2013-04-01 03:00:00"));
    CountingBuffer_p counter1(new CountingBuffer(sensor, time1));
    CountingBuffer_p counter2(new CountingBuffer(sensor, time2));
    CountingBuffer_p counter3(new CountingBuffer(sensor, time3));
    counter1->postRequest(ca);
    counter2->postRequest(ca);
    counter3->postRequest(ca);
    
    EXPECT_EQ(1, counter1->countNew);
    EXPECT_EQ(1, counter2->countNew);
    EXPECT_EQ(2, counter2->size());
    EXPECT_EQ(1, counter3->countNew);
    
    ObsUpdate_pv updated;
    
    ObsData_p obs = counter1->get(SensorTime(sensor, time1.t0()));
    ASSERT_TRUE((bool)obs);
    ObsUpdate_p ou = ca->createUpdate(obs);
    ASSERT_TRUE((bool)ou);
    ou->setCorrected(-12.3);
    updated.push_back(ou);
    
    obs = counter1->get(SensorTime(sensor, s2t("2013-04-01 01:00:00")));
    ASSERT_TRUE((bool)obs);
    ou = ca->createUpdate(obs);
    ASSERT_TRUE((bool)ou);
    ou->setCorrected(-21.0);
    updated.push_back(ou);
    
    ou = ca->createUpdate(SensorTime(sensor, s2t("2013-04-01 00:30:00")));
    ASSERT_TRUE((bool)ou);
    ou->setCorrected(-1.0);
    updated.push_back(ou);
    
    EXPECT_TRUE(ca->storeUpdates(updated));
    EXPECT_EQ(2, counter1->countNew);
    EXPECT_EQ(2, counter2->countNew);
    EXPECT_EQ(1, counter3->countNew);
    
    EXPECT_EQ(1, counter1->countUpdate);
    EXPECT_EQ(1, counter2->countUpdate);
    EXPECT_EQ(0, counter3->countUpdate);
  }

  ca->cleanCache(timeutil::now() + boost::posix_time::hours(1));

  { const TimeSpan time2(s2t("2013-04-01 00:00:00"), s2t("2013-04-01 01:00:00"));
    CountingBuffer_p counter2(new CountingBuffer(sensor, time2));
    counter2->postRequest(ca);
    EXPECT_EQ(3, counter2->size());
  }
}

namespace {
class BadSql : public ObsFilter {
public:
  virtual QString acceptingSql(const QString&, const TimeSpan&) const
    { return "AND the fish have NULL names"; }
};
}

TEST(CachingAccessTest, MalformedSqlInQuery)
{
  SqliteAccess_p sqla(new SqliteAccess);
  sqla->insertDataFromFile(std::string(TEST_SOURCE_DIR)+"/data_18210_20130410.txt");
  CachingAccess_p ca(new CachingAccess(sqla));

  const Sensor sensor(18210, 211, 0, 0, 514);
  const TimeSpan time1(s2t("2013-04-01 00:00:00"), s2t("2013-04-01 03:00:00"));
  CountingBuffer_p counter1(new CountingBuffer(sensor, time1));

  ASSERT_NO_THROW(counter1->postRequest(ca));
}

TEST(CachingAccessTest, ThreadingRequest)
{
  SqliteAccess_p sqla(new SqliteAccess(true));
  sqla->insertDataFromFile(std::string(TEST_SOURCE_DIR)+"/data_18210_20130410.txt");
  CachingAccess_p ca(new CachingAccess(sqla));

  const Sensor sensor(18210, 211, 0, 0, 514);

  // this assumes that SqliteAccess is syncronous
  { const TimeSpan time(s2t("2013-04-01 00:00:00"), s2t("2013-04-03 00:00:00"));
    CountingBuffer_p counter(new CountingBuffer(sensor, time));
    counter->syncRequest(ca);
    EXPECT_EQ(1, sqla->countPost());
    EXPECT_LE(1, counter->countNew);
    EXPECT_EQ(2*24 + 1, counter->size());
    EXPECT_EQ(1, counter->countComplete);
  }
}
