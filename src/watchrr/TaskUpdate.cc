
#include "TaskUpdate.hh"

#include "TaskData.hh"

namespace {
int extractTasks(ObsData_p obs)
{
  if (TaskData_p tobs = boost::dynamic_pointer_cast<TaskData>(obs))
    return tobs->allTasks();
  else
    return 0;
}
} // anonymous namespace

TaskUpdate::TaskUpdate(const SensorTime& st)
  : KvalobsUpdate(st)
  , mTasks(0)
{
}

TaskUpdate::TaskUpdate(ObsData_p obs)
  : KvalobsUpdate(obs)
  , mTasks(extractTasks(obs))
{
}
