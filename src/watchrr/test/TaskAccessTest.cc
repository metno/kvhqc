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


#include "TaskAccess.hh"
#include "TaskData.hh"
#include "TaskUpdate.hh"

#include "common/test/FakeKvApp.hh"
#include "common/test/TestHelpers.hh"

namespace /* anonymous */ {
bool hasTasks(ObsData_p obs)
{
  if (TaskData_p td = std::dynamic_pointer_cast<TaskData>(obs))
    return td->hasTasks();
  else
    return false;
}
bool hasTask(ObsData_p obs, int task)
{
  if (TaskData_p td = std::dynamic_pointer_cast<TaskData>(obs))
    return td->hasTask(task);
  else
    return false;
}
} // anonymous namespace

TEST(TaskAccessTest, Gap)
{
  const Sensor s0(45420, 110, 0, 0, 302);
  const timeutil::ptime t0 = s2t("2012-10-13 06:00:00");
  const SensorTime st0(s0, t0);

  FakeKvApp fa;
  fa.insertStation = s0.stationId;
  fa.insertParam = s0.paramId;
  fa.insertType = s0.typeId;

  fa.insertData("2012-10-12 06:00:00",  -32767.0,       2.0, "0000001000007000", "QC1-7-110,QC2N_xx,QC2-redist,QC1-7-110");
  // fa.insertData("2012-10-13 06:00:00",  -32767.0,       2.0, "0000001000007000", "QC1-7-110,QC2N_xx,QC2-redist,QC1-7-110");
  fa.insertData("2012-10-14 06:00:00",  -32767.0,       2.0, "0000001000007000", "QC1-7-110,QC2N_xx,QC2-redist,QC1-7-110");

  TaskAccess_p tacc = std::make_shared<TaskAccess>(fa.obsAccess());

  ASSERT_FALSE(tacc->findE(st0));

  { tacc->newVersion();
    ObsUpdate_pv updates;

    TaskUpdate_p up = std::dynamic_pointer_cast<TaskUpdate>(tacc->createUpdate(st0));
    ASSERT_TRUE((bool)up);

    up->addTask(4);
    updates.push_back(up);
    ASSERT_TRUE(tacc->storeUpdates(updates));
  }
    
  ObsData_p obs = tacc->findE(st0);
  ASSERT_TRUE((bool)obs);
  ASSERT_TRUE(hasTask(obs, 4));
}
