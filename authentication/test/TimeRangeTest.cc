
#include "TimeRange.hh"
#include <gtest/gtest.h>

namespace {

inline timeutil::ptime s2t(const std::string& txt)
{ return timeutil::from_iso_extended_string(txt); }

const TimeRange t1(s2t("2012-10-01 00:00:00"), s2t("2012-12-01 00:00:00"));

const TimeRange t2(s2t("2012-09-01 00:00:00"), s2t("2012-11-01 00:00:00"));
const TimeRange t3(s2t("2012-11-01 00:00:00"), s2t("2013-01-01 00:00:00"));
const TimeRange t4(s2t("2012-10-10 00:00:00"), s2t("2012-10-20 00:00:00"));
const TimeRange t5(s2t("2012-10-10 00:00:00"), timeutil::ptime());
const TimeRange t6(timeutil::ptime(),          s2t("2012-10-20 00:00:00"));

}

TEST(TimeRangeTest, Intersection)
{
  EXPECT_EQ(TimeRange(t1.t0(), t2.t1()), t1.intersection(t2));
  EXPECT_EQ(TimeRange(t3.t0(), t1.t1()), t1.intersection(t3));

  EXPECT_EQ(t4, t1.intersection(t4));

  EXPECT_EQ(TimeRange(t5.t0(), t1.t1()), t1.intersection(t5));
  EXPECT_EQ(TimeRange(t1.t0(), t6.t1()), t1.intersection(t6));

  EXPECT_EQ(TimeRange(t5.t0(), t6.t1()), t5.intersection(t6));

  EXPECT_EQ(TimeRange(), t3.intersection(t4));

  EXPECT_TRUE(t2.intersection(t3).closed());
}

TEST(TimeRangeTest, Open)
{
  EXPECT_FALSE(t4.open());

  EXPECT_TRUE(t5.open());
  EXPECT_TRUE(t6.open());

  EXPECT_FALSE(TimeRange().open());
}

TEST(TimeRangeTest, Closed)
{
  EXPECT_TRUE(t4.closed());

  EXPECT_FALSE(t5.closed());
  EXPECT_FALSE(t6.closed());

  EXPECT_FALSE(TimeRange().closed());
}

TEST(TimeRangeTest, Undef)
{
  EXPECT_FALSE(t4.undef());

  EXPECT_FALSE(t5.undef());
  EXPECT_FALSE(t6.undef());

  EXPECT_TRUE(TimeRange().undef());
}

