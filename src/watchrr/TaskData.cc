
#include "TaskData.hh"

#include "common/Tasks.hh"

bool TaskData::hasRequiredTasks() const
{
  return (allTasks() & tasks::REQUIRED_TASK_MASK) != 0;
}
