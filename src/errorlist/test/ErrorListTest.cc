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


#include "errorlist/ErrorFilter.hh"

#include "common/KvMetaDataBuffer.hh"
#include "common/KvServiceHelper.hh"
#include "common/TimeBuffer.hh"

#define LOAD_DECL_ONLY
#include "load_3200_20141008.cc"

#define MILOGGER_CATEGORY "kvhqc.test.ErrorListTest"
#include "common/ObsLogging.hh"

namespace /* anonymous */ {

struct TestSetup {
  std::shared_ptr<FakeKvApp> fa;
  KvServiceHelper kvsh;
  KvMetaDataBuffer kvmdbuf;

  TestSetup(bool useThread = false)
      : fa(std::make_shared<FakeKvApp>(useThread))
      , kvsh(fa)
  {
    kvmdbuf.setHandler(fa->obsAccess()->handler());
  }
};

} // anonymous

TEST(ErrorListTest, Filter1)
{
  TestSetup ts;
  load_3200_20141008(*ts.fa);
  ts.kvmdbuf.reload();

  const Sensor sensor1(Sensor(2650, 61, 0, 0, 330));
  Sensor_s sensors;
  sensors.insert(sensor1);

  ErrorFilter_p ef(new ErrorFilter(false));
  TimeBuffer_p b = std::make_shared<TimeBuffer>(sensors, t_3200_20141008(), ef);
  b->syncRequest(ts.fa->obsAccess());

  EXPECT_EQ(3, b->size());

  const ObsData_pv d(b->data().begin(), b->data().end());
  EXPECT_TRUE(eq_Sensor()(sensor1, d[0]->sensorTime().sensor));
  EXPECT_TRUE(eq_Sensor()(sensor1, d[1]->sensorTime().sensor));
  EXPECT_TRUE(eq_Sensor()(sensor1, d[2]->sensorTime().sensor));
  EXPECT_EQ(s2t("2014-10-07 06:00:00"), d[0]->sensorTime().time);
  EXPECT_EQ(s2t("2014-10-07 07:00:00"), d[1]->sensorTime().time);
  EXPECT_EQ(s2t("2014-10-07 08:00:00"), d[2]->sensorTime().time);
}

TEST(ErrorListTest, Filter1Salen)
{
  TestSetup ts;
  load_3200_20141008(*ts.fa);
  ts.kvmdbuf.reload();

  const Sensor sensor1(Sensor(2650, 61, 0, 0, 330));
  Sensor_s sensors;
  sensors.insert(sensor1);

  ErrorFilter_p ef(new ErrorFilter(true));
  TimeBuffer_p b = std::make_shared<TimeBuffer>(sensors, t_3200_20141008(), ef);
  b->syncRequest(ts.fa->obsAccess());

  EXPECT_EQ(0, b->size());
}

TEST(ErrorListTest, FilterMany)
{
  TestSetup ts;
  load_3200_20141008(*ts.fa);
  ts.kvmdbuf.reload();

  Sensor_s sensors;
  sensors.insert(Sensor(2650, 61, 0, 0, 330));
  sensors.insert(Sensor(2650, 81, 0, 0, 330));
  sensors.insert(Sensor(2650, 211, 0, 0, 330));
  sensors.insert(Sensor(2650, 213, 0, 0, 330));
  sensors.insert(Sensor(2650, 215, 0, 0, 330));
  sensors.insert(Sensor(2650, 262, 0, 0, 330));
  sensors.insert(Sensor(4460, 61, 0, 0, 342));
  sensors.insert(Sensor(4460, 61, 0, 0, 506));
  sensors.insert(Sensor(4460, 81, 0, 0, 342));
  sensors.insert(Sensor(4460, 81, 0, 0, 506));
  sensors.insert(Sensor(4460, 211, 0, 0, 342));
  sensors.insert(Sensor(4460, 213, 0, 0, 342));
  sensors.insert(Sensor(4460, 215, 0, 0, 342));
  sensors.insert(Sensor(4460, 262, 0, 0, 342));
  sensors.insert(Sensor(5590, 61, 0, 0, 330));
  sensors.insert(Sensor(5590, 81, 0, 0, 330));
  sensors.insert(Sensor(5590, 211, 0, 0, 330));
  sensors.insert(Sensor(5590, 213, 0, 0, 330));
  sensors.insert(Sensor(5590, 215, 0, 0, 330));
  sensors.insert(Sensor(5590, 262, 0, 0, 330));
  sensors.insert(Sensor(7420, 61, 0, 0, 501));
  sensors.insert(Sensor(7420, 61, 0, 0, 506));
  sensors.insert(Sensor(7420, 81, 0, 0, 501));
  sensors.insert(Sensor(7420, 81, 0, 0, 506));
  sensors.insert(Sensor(7420, 211, 0, 0, 501));
  sensors.insert(Sensor(7420, 211, 0, 0, 506));
  sensors.insert(Sensor(7420, 213, 0, 0, 501));
  sensors.insert(Sensor(7420, 215, 0, 0, 501));
  sensors.insert(Sensor(7420, 262, 0, 0, 501));
  sensors.insert(Sensor(18020, 211, 0, 0, 514));
  sensors.insert(Sensor(18020, 213, 0, 0, 514));
  sensors.insert(Sensor(18020, 215, 0, 0, 514));

  ErrorFilter_p ef(new ErrorFilter(false));
  TimeBuffer_p b = std::make_shared<TimeBuffer>(sensors, t_3200_20141008(), ef);
  b->syncRequest(ts.fa->obsAccess());

  EXPECT_EQ(206, b->size());
}
