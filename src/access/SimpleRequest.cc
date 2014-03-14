
#include "SimpleRequest.hh"

#define MILOGGER_CATEGORY "kvhqc.SimpleRequest"
#include "common/ObsLogging.hh"

SimpleRequest::SimpleRequest(SimpleBuffer* buffer, const Sensor_s& sensors, const TimeSpan& timeSpan, ObsFilter_p filter)
  : BaseRequest(sensors, timeSpan, filter)
  , mBuffer(buffer)
{
}

void SimpleRequest::completed(bool failed)
{
  METLIBS_LOG_SCOPE();
  mBuffer->completed(failed);
}

void SimpleRequest::newData(const ObsData_pv& data)
{
  METLIBS_LOG_SCOPE();
  mBuffer->newData(data);
}

void SimpleRequest::updateData(const ObsData_pv& data)
{
  METLIBS_LOG_SCOPE();
  mBuffer->updateData(data);
}

void SimpleRequest::dropData(const SensorTime_v& dropped)
{
  METLIBS_LOG_SCOPE();
  mBuffer->dropData(dropped);
}
