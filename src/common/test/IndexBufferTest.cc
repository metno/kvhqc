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

  {
    IndexBuffer_p buffer = std::make_shared<IndexBuffer>(3600, sensors, time);
    buffer->syncRequest(sqla);
    EXPECT_EQ(7u, buffer->size());
  }
}
