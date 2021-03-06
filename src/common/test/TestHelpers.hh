
#ifndef COMMON_TEST_TESTHELPERS_HH
#define COMMON_TEST_TESTHELPERS_HH 1

#include "common/ObsData.hh"
#include "common/Sensor.hh"
#include "common/TimeRange.hh"

#include <gtest/gtest.h>
#include <memory>
#include <string>

inline timeutil::ptime s2t(const std::string& t)
{ return timeutil::from_iso_extended_string(t); }

::testing::AssertionResult AssertCorrControl(const char* ec_expr, const char* eci_expr, const char* a_expr,
                                             float ec, const std::string& eci, const ObsDataPtr& a);

#define EXPECT_CORR_CONTROL(ec, eci, a) EXPECT_PRED_FORMAT3(AssertCorrControl, ec, eci, a)
#define ASSERT_CORR_CONTROL(ec, eci, a) ASSERT_PRED_FORMAT3(AssertCorrControl, ec, eci, a)

#endif // COMMON_TEST_TESTHELPERS_HH
