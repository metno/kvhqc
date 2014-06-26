
#include "TaskAccess.hh"

#include "TaskData.hh"
#include "TaskUpdate.hh"

#include "common/SingleObsBuffer.hh"

#include <boost/make_shared.hpp>

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
  return boost::make_shared<TaskUpdate>(data);
}

ObsUpdate_p TaskAccess::createUpdate(const SensorTime& sensorTime)
{
  return boost::make_shared<TaskUpdate>(sensorTime);
}

bool TaskAccess::storeToBackend()
{
  return EditAccess::storeToBackend();
}

KvalobsData_p TaskAccess::createDataForUpdate(KvalobsUpdate_p update, const timeutil::ptime& tbtime)
{
  KvalobsData_p kd = EditAccess::createDataForUpdate(update, tbtime);
  return boost::make_shared<TaskData>(kd);
}

ObsData_p TaskAccess::findE(const SensorTime& st)
{
  SingleObsBuffer ob(st);
  ob.syncRequest(shared_from_this());
  return ob.get();
}
