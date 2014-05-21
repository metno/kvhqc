
#include "KvHelpers.hh"
#include <kvalobs/kvDataOperations.h>
#include <gtest/gtest.h>

TEST(KvHelpersTest, getFlagText)
{
  ASSERT_EQ("", Helpers::getFlagText(std::string("0000000000000000")).toStdString());

  {
    kvalobs::kvControlInfo ci;
    ci.set(kvalobs::flag::ftime, 1);
    ASSERT_EQ(1, ci.flag(kvalobs::flag::ftime));
    ASSERT_EQ("ftime=1", Helpers::getFlagText(ci).toStdString());
  }

  {
    kvalobs::kvControlInfo ci;
    ci.set(kvalobs::flag::fd, 1);
    ASSERT_EQ(1, ci.flag(kvalobs::flag::fd));
    ASSERT_EQ("", Helpers::getFlagText(ci).toStdString());

    ci.set(kvalobs::flag::fd, 2);
    ASSERT_EQ(2, ci.flag(kvalobs::flag::fd));
    ASSERT_EQ("fd=2", Helpers::getFlagText(ci).toStdString());
  }
}

TEST(KvHelpersTest, formatValue)
{
  ASSERT_EQ("12.1", Helpers::formatValue(12.071f).toStdString());
  ASSERT_EQ("-1.1", Helpers::formatValue(-1.071f).toStdString());
}

TEST(KvHelpersTest, sensorTimeToString)
{
  const SensorTime st1(Sensor(18700, 211, 0, 0, 330), timeutil::from_iso_extended_string("2014-05-15 15:00:00"));
  EXPECT_EQ("stationid=18700;level=0;sensornr=0;typeid=330;paramid=211;time=2014-05-15 15:00:00;", Helpers::sensorTimeToString(st1).toStdString());
}

TEST(KvHelpersTest, sensorTimeFromString)
{
  const SensorTime st1 = Helpers::sensorTimeFromString("stationid=18700;level=0;sensornr=0;typeid=330;paramid=215;time=2014-05-15 15:00:00;");
  EXPECT_EQ(18700, st1.sensor.stationId);
  EXPECT_EQ(0,     st1.sensor.level);
  EXPECT_EQ(0,     st1.sensor.sensor);
  EXPECT_EQ(330,   st1.sensor.typeId);
  EXPECT_EQ(215,   st1.sensor.paramId);
}
