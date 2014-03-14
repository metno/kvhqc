
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

ObsData_p TimeBuffer::get(const SensorTime& st) const
{
  const ObsDataByTime_ps::const_iterator it = find(st);
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
    const ObsDataByTime_ps::iterator it = find((*itD)->sensorTime());
    if (it != mData.end()) {
      mData.erase(it);
      mData.insert(*itD);
    }
  }
}

void TimeBuffer::onDropData(const SensorTime_v& dropped)
{
  for (SensorTime_v::const_iterator itS = dropped.begin(); itS != dropped.end(); ++itS) {
    const ObsDataByTime_ps::iterator it = find(*itS);
    if (it != mData.end())
      mData.erase(it);
  }
}

TimeBuffer::ObsDataByTime_ps::iterator TimeBuffer::find(const SensorTime& st)
{
  const ObsDataByTime_ps::iterator it = std::lower_bound(mData.begin(), mData.end(), st, ObsData_by_time());
  if (it != mData.end() and eq_SensorTime()(st, (*it)->sensorTime()))
    return it;
  return mData.end();
}

TimeBuffer::ObsDataByTime_ps::const_iterator TimeBuffer::find(const SensorTime& st) const
{
  const ObsDataByTime_ps::const_iterator it = std::lower_bound(mData.begin(), mData.end(), st, ObsData_by_time());
  if (it != mData.end() and eq_SensorTime()(st, (*it)->sensorTime()))
    return it;
  return mData.end();
}
