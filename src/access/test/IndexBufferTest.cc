
#include "IndexBuffer.hh"
#include "SqliteAccess.hh"

#include <boost/make_shared.hpp>

#include <gtest/gtest.h>

inline timeutil::ptime s2t(const std::string& t)
{ return timeutil::from_iso_extended_string(t); }

TEST(IndexBufferTest, Basic)
{
  SqliteAccess_p sqla(new SqliteAccess(false));
  sqla->insertDataFromFile(std::string(TEST_SOURCE_DIR)+"/../../common/test/data_18210_20130410.txt");

  const Sensor sensor(18210, 211, 0, 0, 514);
  const TimeRange time(s2t("2013-04-01 00:00:00"), s2t("2013-04-01 06:00:00"));

  { IndexBuffer_p buffer = boost::make_shared<IndexBuffer>(3600, sensor, time);
    buffer->syncRequest(sqla);
    EXPECT_EQ(7, buffer->size());
  }
}
