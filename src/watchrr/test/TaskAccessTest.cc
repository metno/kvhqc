
#include "TaskAccess.hh"
#include "TaskData.hh"
#include "TaskUpdate.hh"

#include "common/test/FakeKvApp.hh"
#include "common/test/TestHelpers.hh"

namespace /* anonymous */ {
bool hasTasks(ObsData_p obs)
{
  if (TaskData_p td = boost::dynamic_pointer_cast<TaskData>(obs))
    return td->hasTasks();
  else
    return false;
}
bool hasTask(ObsData_p obs, int task)
{
  if (TaskData_p td = boost::dynamic_pointer_cast<TaskData>(obs))
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

  TaskAccess_p tacc = boost::make_shared<TaskAccess>(fa.obsAccess());

  ASSERT_FALSE(tacc->findE(st0));

  { tacc->newVersion();
    ObsUpdate_pv updates;
    
    TaskUpdate_p up = boost::dynamic_pointer_cast<TaskUpdate>(tacc->createUpdate(st0));
    ASSERT_TRUE(up);
    
    up->addTask(4);
    updates.push_back(up);
    ASSERT_TRUE(tacc->storeUpdates(updates));
  }
    
  ObsData_p obs = tacc->findE(st0);
  ASSERT_TRUE(obs);
  ASSERT_TRUE(hasTask(obs, 4));
}
