
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

IndexBuffer::IndexBuffer(int stepSeconds, const Sensor& sensor, const TimeSpan& timeSpan, ObsFilter_p filter)
  : SimpleBuffer(sensor, timeSpan, filter)
  , mStepSeconds(stepSeconds)
  , mData(steps(timeSpan, mStepSeconds), ObsData_p())
{
  METLIBS_LOG_SCOPE(LOGVAL(mData.size()));
}

IndexBuffer::IndexBuffer(int stepSeconds, SignalRequest_p request)
  : SimpleBuffer(request)
  , mStepSeconds(stepSeconds)
  , mData(steps(request->timeSpan(), mStepSeconds), ObsData_p())
{
  METLIBS_LOG_SCOPE(LOGVAL(mData.size()));
}

void IndexBuffer::newData(const ObsData_pv& data)
{
  METLIBS_LOG_SCOPE();
  for (ObsData_pv::const_iterator it = data.begin(); it != data.end(); ++it) {
    const int idx = findIndex(*it);
    METLIBS_LOG_DEBUG(LOGVAL((*it)->sensorTime().time) << LOGVAL(idx));
    if (idx >= 0)
      mData[idx] = *it;
  }
}

void IndexBuffer::updateData(const ObsData_pv& data)
{
  METLIBS_LOG_SCOPE();
  newData(data);
}

void IndexBuffer::dropData(const SensorTime_v& dropped)
{
  METLIBS_LOG_SCOPE();
  for (SensorTime_v::const_iterator it = dropped.begin(); it != dropped.end(); ++it) {
    const int idx = findIndex(it->time);
    if (idx >= 0)
      mData[idx] = ObsData_p();
  }
}

int IndexBuffer::findIndex(ObsData_p obs) const
{
  return findIndex(obs->sensorTime().time);
}

int IndexBuffer::findIndex(const Time& time) const
{
  const div_t d = std::div(secondsSinceStart(request()->timeSpan(), time), mStepSeconds);
  if (d.rem == 0 and d.quot < (int)mData.size())
    return d.quot;
  else
    return -1;
}
