
#ifndef COMMON_TEST_TESTHELPERS_HH
#define COMMON_TEST_TESTHELPERS_HH 1

#include "common/Functors.hh"
#include "common/ObsData.hh"
#include "common/Sensor.hh"
#include "common/TimeSpan.hh"

#include <gtest/gtest.h>
#include <boost/make_shared.hpp>
#include <string>

inline timeutil::ptime s2t(const std::string& t)
{ return timeutil::from_iso_extended_string(t); }

::testing::AssertionResult AssertCorrControl(const char* ec_expr, const char* eci_expr, const char* a_expr,
                                             float ec, const std::string& eci, const ObsData_p& a);

#define EXPECT_CORR_CONTROL(ec, eci, a) EXPECT_PRED_FORMAT3(AssertCorrControl, ec, eci, a)
#define ASSERT_CORR_CONTROL(ec, eci, a) ASSERT_PRED_FORMAT3(AssertCorrControl, ec, eci, a)

#endif // COMMON_TEST_TESTHELPERS_HH
