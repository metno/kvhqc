
#include "CachingAccess.hh"
#include "IndexBuffer.hh"
#include "SqliteAccess.hh"
#include "SyncRequest.hh"
#include "TimeBuffer.hh"

#include <boost/make_shared.hpp>

#include <gtest/gtest.h>

inline timeutil::ptime s2t(const std::string& t)
{ return timeutil::from_iso_extended_string(t); }

TEST(SyncRequestTest, NoThread)
{
  SqliteAccess_p sqla(new SqliteAccess(false));
  sqla->insertDataFromFile(std::string(TEST_SOURCE_DIR)+"/../../common/test/data_18210_20130410.txt");

  const Sensor sensor(18210, 211, 0, 0, 514);
  const TimeRange time(s2t("2013-04-01 00:00:00"), s2t("2013-04-03 00:00:00"));

  { TimeBuffer_p buffer = boost::make_shared<TimeBuffer>(sensor, time);
    buffer->syncRequest(sqla);
    EXPECT_EQ(2*24 + 1, buffer->size());
  }
}

TEST(SyncRequestTest, Thread)
{
  SqliteAccess_p sqla(new SqliteAccess(true));
  sqla->insertDataFromFile(std::string(TEST_SOURCE_DIR)+"/../../common/test/data_18210_20130410.txt");

  const Sensor sensor(18210, 211, 0, 0, 514);
  const TimeRange time(s2t("2013-04-01 00:00:00"), s2t("2013-04-03 00:00:00"));

  { IndexBuffer_p buffer = boost::make_shared<IndexBuffer>(3600, sensor, time);
    buffer->syncRequest(sqla);
    EXPECT_EQ(2*24 + 1, buffer->size());
  }
}

TEST(SyncRequestTest, Cached)
{
  SqliteAccess_p sqla(new SqliteAccess(true));
  sqla->insertDataFromFile(std::string(TEST_SOURCE_DIR)+"/../../common/test/data_18210_20130410.txt");

  CachingAccess_p ca(new CachingAccess(sqla));

  const Sensor sensor(18210, 211, 0, 0, 514);

  { const TimeRange time(s2t("2013-04-01 00:00:00"), s2t("2013-04-03 00:00:00"));
    TimeBuffer_p buffer = boost::make_shared<TimeBuffer>(sensor, time);
    buffer->syncRequest(ca);
    EXPECT_EQ(1, sqla->countPost());
    EXPECT_EQ(2*24 + 1, buffer->size());
  }

  { const TimeRange time(s2t("2013-04-01 00:00:00"), s2t("2013-04-02 00:00:00"));
    TimeBuffer_p buffer = boost::make_shared<TimeBuffer>(sensor, time);
    buffer->syncRequest(ca);
    EXPECT_EQ(1, sqla->countPost());
    EXPECT_EQ(1*24 + 1, buffer->size());
  }
}
