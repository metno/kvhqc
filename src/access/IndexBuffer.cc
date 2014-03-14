
#include "IndexBuffer.hh"

#include "SimpleRequest.hh"

#include <cstdlib> // for 'div'

#define MILOGGER_CATEGORY "kvhqc.IndexBuffer"
#include "common/ObsLogging.hh"

// ========================================================================

namespace /*anonymous*/ {

int secondsSinceStart(const TimeSpan& ts, const Time& t)
{
  return (t - ts.t0()).total_seconds();
}

int duration(const TimeSpan& ts)
{
  return secondsSinceStart(ts, ts.t1());
}

int steps(const TimeSpan& ts, int step)
{
  const div_t d = std::div(duration(ts), step);
  if (d.rem != 0)
    return d.quot;
  else
    return d.quot + 1;
}

} // namespace anonymous

// ========================================================================

IndexBuffer::IndexBuffer(int stepSeconds, const Sensor_s& sensors, const TimeSpan& timeSpan, ObsFilter_p filter)
  : SimpleBuffer(sensors, timeSpan, filter)
  , mStepSeconds(stepSeconds)
  , mSize(steps(timeSpan, mStepSeconds))
{
  METLIBS_LOG_SCOPE();
  const ObsData_pv empty(mSize, ObsData_p());
  for (Sensor_s::const_iterator itS = sensors.begin(); itS != sensors.end(); ++itS)
    mData.insert(std::make_pair(*itS, empty));
}

IndexBuffer::IndexBuffer(int stepSeconds, SignalRequest_p request)
  : SimpleBuffer(request)
  , mStepSeconds(stepSeconds)
  , mSize(steps(request->timeSpan(), mStepSeconds))
{
  METLIBS_LOG_SCOPE();
  const ObsData_pv empty(mSize, ObsData_p());
  const Sensor_s& sensors = request->sensors();
  for (Sensor_s::const_iterator itS = sensors.begin(); itS != sensors.end(); ++itS)
    mData.insert(std::make_pair(*itS, empty));
}

void IndexBuffer::onNewData(const ObsData_pv& data)
{
  METLIBS_LOG_SCOPE();
  for (ObsData_pv::const_iterator it = data.begin(); it != data.end(); ++it) {
    const SensorTime& st = (*it)->sensorTime();
    const int idx = findIndex(st);
    if (idx < 0)
      continue;
    const data_m::iterator itD = mData.find(st.sensor);
    if (itD != mData.end())
      itD->second[idx] = *it;
  }
}

void IndexBuffer::onUpdateData(const ObsData_pv& data)
{
  METLIBS_LOG_SCOPE();
  newData(data);
}

void IndexBuffer::onDropData(const SensorTime_v& dropped)
{
  METLIBS_LOG_SCOPE();
  for (SensorTime_v::const_iterator it = dropped.begin(); it != dropped.end(); ++it) {
    const int idx = findIndex(*it);
    if (idx < 0)
      continue;
    const data_m::iterator itD = mData.find(it->sensor);
    if (itD != mData.end())
      itD->second[idx] = ObsData_p();
  }
}

int IndexBuffer::findIndex(ObsData_p obs) const
{
  return findIndex(obs->sensorTime());
}

int IndexBuffer::findIndex(const SensorTime& st) const
{
  const div_t d = std::div(secondsSinceStart(request()->timeSpan(), st.time), mStepSeconds);
  if (d.rem == 0 and d.quot < mSize)
    return d.quot;
  else
    return -1;
}
