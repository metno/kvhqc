/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2018 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: kvalobs-dev@met.no

  This file is part of HQC

  HQC is free software; you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  HQC is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General Public License
  along with HQC; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/


#include "Helpers.hh"
#include "stringutil.hh"
#include <gtest/gtest.h>
#include <stdexcept>

TEST(HelpersTest, roundDecimals)
{
  ASSERT_FLOAT_EQ(12.1f, Helpers::roundDecimals(12.1f, 1));
  ASSERT_FLOAT_EQ(12.0f, Helpers::roundDecimals(12.1f, 0));
  ASSERT_FLOAT_EQ(10.0f, Helpers::roundDecimals(12.1f, -1));

  ASSERT_FLOAT_EQ(-3.6f, Helpers::roundDecimals(-3.6f, 1));
  ASSERT_FLOAT_EQ(-4.0f, Helpers::roundDecimals(-3.6f, 0));
  ASSERT_FLOAT_EQ( 0.0f, Helpers::roundDecimals(-3.6f, -1));
}

TEST(HelpersTest, parseFloat)
{
  ASSERT_THROW(Helpers::parseFloat("12.1", 0), std::runtime_error);
  try {
    Helpers::parseFloat("12.1", 1);
  } catch(std::runtime_error& e) {
    FAIL() << e.what();
  }
  ASSERT_NO_THROW(Helpers::parseFloat("12", 0));

  ASSERT_THROW(Helpers::parseFloat("-3.1", 0), std::runtime_error);
  ASSERT_NO_THROW(Helpers::parseFloat("-3.1", 1));
  ASSERT_NO_THROW(Helpers::parseFloat("-3", 0));
}

TEST(HelpersTest, appendedText)
{
  ASSERT_EQ("wo, ho", Helpers::appendedText("wo", "ho", ", ").toStdString());
  ASSERT_EQ("wo", Helpers::appendedText("wo", "", ", ").toStdString());
  ASSERT_EQ("ho", Helpers::appendedText("", "ho", ", ").toStdString());
  ASSERT_EQ("", Helpers::appendedText("", "", ", ").toStdString());
}
