
#include "IndexBuffer.hh"
#include "SqliteAccess.hh"

#include "util/make_set.hh"

#include <gtest/gtest.h>

inline timeutil::ptime s2t(const std::string& t)
{ return timeutil::from_iso_extended_string(t); }

TEST(IndexBufferTest, Basic)
{
  SqliteAccess_p sqla(new SqliteAccess(false));
  sqla->insertDataFromFile(std::string(TEST_SOURCE_DIR)+"/../../common/test/data_18210_20130410.txt");

  const Sensor_s sensors = make_set<Sensor_s>(Sensor(18210, 211, 0, 0, 514));
  const TimeSpan time(s2t("2013-04-01 00:00:00"), s2t("2013-04-01 06:00:00"));

  { IndexBuffer_p buffer = std::make_shared<IndexBuffer>(3600, sensors, time);
    buffer->syncRequest(sqla);
    EXPECT_EQ(7, buffer->size());
  }
}
