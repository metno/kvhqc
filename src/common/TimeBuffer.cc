
#include "TimeBuffer.hh"

#define MILOGGER_CATEGORY "kvhqc.TimeBuffer"
#include "common/ObsLogging.hh"

// ========================================================================

namespace /*anonymous*/ {

} // namespace anonymous

// ========================================================================

TimeBuffer::TimeBuffer(const Sensor_s& sensors, const TimeSpan& timeSpan, ObsFilter_p filter)
  : SimpleBuffer(sensors, timeSpan, filter)
{
}

TimeBuffer::TimeBuffer(SignalRequest_p request)
  : SimpleBuffer(request)
{
}

Time_s TimeBuffer::times() const
{
  Time_s times;
  for (ObsData_ps_ST::iterator itD = mData.begin(); itD != mData.end(); ++itD)
    times.insert((*itD)->sensorTime().time);
  return times;
}

ObsData_p TimeBuffer::get(const SensorTime& st) const
{
  const ObsData_ps_ST::const_iterator it = find(st);
  if (it != mData.end())
    return *it;
  return ObsData_p();
}

void TimeBuffer::onNewData(const ObsData_pv& data)
{
  METLIBS_LOG_SCOPE();
  mData.insert(data.begin(), data.end());
}

void TimeBuffer::onUpdateData(const ObsData_pv& data)
{
  METLIBS_LOG_SCOPE();
  for (ObsData_pv::const_iterator itD = data.begin(); itD != data.end(); ++itD) {
    const ObsData_ps_ST::iterator it = find((*itD)->sensorTime());
    if (it != mData.end()) {
      mData.erase(it);
      mData.insert(*itD);
    }
  }
}

void TimeBuffer::onDropData(const SensorTime_v& dropped)
{
  METLIBS_LOG_SCOPE();
  for (SensorTime_v::const_iterator itS = dropped.begin(); itS != dropped.end(); ++itS) {
    METLIBS_LOG_SCOPE(*itS);
    const ObsData_ps_ST::iterator it = find(*itS);
    if (it != mData.end())
      mData.erase(it);
  }
}

ObsData_ps_ST::iterator TimeBuffer::find(const SensorTime& st)
{
  const ObsData_ps_ST::iterator it = std::lower_bound(mData.begin(), mData.end(), st, ObsData_by_SensorTime());
  if (it != mData.end() and eq_SensorTime()(st, (*it)->sensorTime()))
    return it;
  return mData.end();
}

ObsData_ps_ST::const_iterator TimeBuffer::find(const SensorTime& st) const
{
  const ObsData_ps_ST::const_iterator it = std::lower_bound(mData.begin(), mData.end(), st, ObsData_by_SensorTime());
  if (it != mData.end() and eq_SensorTime()(st, (*it)->sensorTime()))
    return it;
  return mData.end();
}
