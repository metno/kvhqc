
#include "CachingAccess.hh"
#include "IndexBuffer.hh"
#include "SqliteAccess.hh"
#include "SyncRequest.hh"
#include "TimeBuffer.hh"

#include "util/make_set.hh"

#include <boost/make_shared.hpp>

#include <gtest/gtest.h>

inline timeutil::ptime s2t(const std::string& t)
{ return timeutil::from_iso_extended_string(t); }

TEST(SyncRequestTest, NoThread)
{
  SqliteAccess_p sqla(new SqliteAccess(false));
  sqla->insertDataFromFile(std::string(TEST_SOURCE_DIR)+"/data_18210_20130410.txt");

  const Sensor_s sensors = make_set<Sensor_s>(Sensor(18210, 211, 0, 0, 514));
  const TimeSpan time(s2t("2013-04-01 00:00:00"), s2t("2013-04-03 00:00:00"));

  { TimeBuffer_p buffer = boost::make_shared<TimeBuffer>(sensors, time);
    buffer->syncRequest(sqla);
    EXPECT_EQ(2*24 + 1, buffer->size());
  }
}

TEST(SyncRequestTest, Thread)
{
  SqliteAccess_p sqla(new SqliteAccess(true));
  sqla->insertDataFromFile(std::string(TEST_SOURCE_DIR)+"/data_18210_20130410.txt");

  const Sensor_s sensors = make_set<Sensor_s>(Sensor(18210, 211, 0, 0, 514));
  const TimeSpan time(s2t("2013-04-01 00:00:00"), s2t("2013-04-03 00:00:00"));

  { IndexBuffer_p buffer = boost::make_shared<IndexBuffer>(3600, sensors, time);
    buffer->syncRequest(sqla);
    EXPECT_EQ(2*24 + 1, buffer->size());
  }
}

TEST(SyncRequestTest, Cached)
{
  SqliteAccess_p sqla(new SqliteAccess(true));
  sqla->insertDataFromFile(std::string(TEST_SOURCE_DIR)+"/data_18210_20130410.txt");

  CachingAccess_p ca(new CachingAccess(sqla));

  const Sensor_s sensors = make_set<Sensor_s>(Sensor(18210, 211, 0, 0, 514));

  { const TimeSpan time(s2t("2013-04-01 00:00:00"), s2t("2013-04-03 00:00:00"));
    TimeBuffer_p buffer = boost::make_shared<TimeBuffer>(sensors, time);
    buffer->syncRequest(ca);
    EXPECT_EQ(1, sqla->countPost());
    EXPECT_EQ(2*24 + 1, buffer->size());
  }

  { const TimeSpan time(s2t("2013-04-01 00:00:00"), s2t("2013-04-02 00:00:00"));
    TimeBuffer_p buffer = boost::make_shared<TimeBuffer>(sensors, time);
    buffer->syncRequest(ca);
    EXPECT_EQ(1, sqla->countPost());
    EXPECT_EQ(1*24 + 1, buffer->size());
  }
}

TEST(SyncRequestTest, CachedMulti)
{
  SqliteAccess_p sqla(new SqliteAccess);
  sqla->insertDataFromFile(std::string(TEST_SOURCE_DIR)+"/data_18700_20140304.txt");
  CachingAccess_p ca(new CachingAccess(sqla));

  const Sensor sensor1(18210, 211, 0, 0, 514);
  const Sensor sensor2(18700, 211, 0, 0, 330);
  const Sensor_s sensors = (SetMaker<Sensor_s>() << sensor1 << sensor2).set();

  { const TimeSpan time(s2t("2014-03-01 00:00:00"), s2t("2014-03-01 06:00:00"));
    TimeBuffer_p buffer = boost::make_shared<TimeBuffer>(make_set<Sensor_s>(sensor1), time);
    buffer->syncRequest(ca);
    EXPECT_EQ(1, sqla->countPost());
    EXPECT_EQ(7, buffer->size());
  }

  { const TimeSpan time(s2t("2014-03-01 03:00:00"), s2t("2014-03-01 09:00:00"));
    TimeBuffer_p buffer = boost::make_shared<TimeBuffer>(make_set<Sensor_s>(sensor2), time);
    buffer->syncRequest(ca);
    EXPECT_EQ(2, sqla->countPost());
    EXPECT_EQ(7, buffer->size());
  }

  { const TimeSpan time(s2t("2014-03-01 00:00:00"), s2t("2014-03-01 09:00:00"));
    TimeBuffer_p buffer = boost::make_shared<TimeBuffer>(sensors, time);
    buffer->syncRequest(ca);
    EXPECT_EQ(4, sqla->countPost());
    EXPECT_EQ(20, buffer->size());
  }
}
