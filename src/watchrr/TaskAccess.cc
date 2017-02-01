
#include "TaskAccess.hh"

#include "TaskData.hh"
#include "TaskUpdate.hh"

#include "common/SingleObsBuffer.hh"

#define MILOGGER_CATEGORY "kvhqc.TaskAccess"
#include "common/ObsLogging.hh"

TaskAccess::TaskAccess(ObsAccess_p backend)
  : EditAccess(backend)
{
}

TaskAccess::~TaskAccess()
{
}

ObsUpdate_p TaskAccess::createUpdate(ObsData_p data)
{
  return std::make_shared<TaskUpdate>(data);
}

ObsUpdate_p TaskAccess::createUpdate(const SensorTime& sensorTime)
{
  return std::make_shared<TaskUpdate>(sensorTime);
}

KvalobsData_p TaskAccess::createDataForUpdate(KvalobsUpdate_p update, const timeutil::ptime& tbtime)
{
  METLIBS_LOG_SCOPE();
  int tasks = 0;
  if (TaskUpdate_p tu = std::dynamic_pointer_cast<TaskUpdate>(update))
    tasks = tu->tasks();
  METLIBS_LOG_DEBUG(LOGVAL(update->sensorTime()) << LOGVAL(tasks));
  KvalobsData_p kd = EditAccess::createDataForUpdate(update, tbtime);
  return std::make_shared<TaskData>(kd, tasks);
}

bool TaskAccess::fillBackendupdate(ObsUpdate_p backendUpdate, ObsData_p currentData)
{
  TaskUpdate_p tu = std::dynamic_pointer_cast<TaskUpdate>(backendUpdate);
  if (TaskData_p td = std::dynamic_pointer_cast<TaskData>(currentData)) {
    if (tu)
      tu->setTasks(td->allTasks());
    else if (td->hasRequiredTasks())
      return false;
  }
  return EditAccess::fillBackendupdate(backendUpdate, currentData);
}

ObsData_p TaskAccess::findE(const SensorTime& st)
{
  SingleObsBuffer ob(st);
  ob.syncRequest(shared_from_this());
  return ob.get();
}
