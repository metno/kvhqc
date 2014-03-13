
#include "TimeBuffer.hh"

#define MILOGGER_CATEGORY "kvhqc.TimeBuffer"
#include "common/ObsLogging.hh"

// ========================================================================

namespace /*anonymous*/ {

struct ObsData_by_time {
  bool operator()(ObsData_p a, ObsData_p b) const
    { return a->sensorTime().time < b->sensorTime().time; }
  bool operator()(ObsData_p a, const Time& b) const
    { return a->sensorTime().time < b; }
  bool operator()(const Time& a, ObsData_p b) const
    { return a < b->sensorTime().time; }
};

} // namespace anonymous

// ========================================================================

TimeBuffer::TimeBuffer(const Sensor& sensor, const TimeSpan& timeSpan, ObsFilter_p filter)
  : SimpleBuffer(sensor, timeSpan, filter)
{
}

TimeBuffer::TimeBuffer(SignalRequest_p request)
  : SimpleBuffer(request)
{
}

ObsData_p TimeBuffer::get(const Time& time) const
{
  const ObsData_pl::const_iterator itGet = findObs(time);
  ObsData_p d = *itGet;
  if (d->sensorTime().time == time)
    return d;
  return ObsData_p();
}

void TimeBuffer::newData(const ObsData_pv& data)
{
  METLIBS_LOG_SCOPE();
  if (not data.empty()) {
    ObsData_pv::const_iterator dataBegin = data.begin();
    const Time& dbTime = (*dataBegin)->sensorTime().time;
    const ObsData_pl::iterator itInsert = findObs(dbTime);
    if (itInsert != mData.end() and dbTime == (*itInsert)->sensorTime().time)
      ++dataBegin;
    mData.insert(itInsert, dataBegin, data.end());
  }
}

void TimeBuffer::updateData(const ObsData_pv& data)
{
  METLIBS_LOG_SCOPE();
  for (ObsData_pv::const_iterator itD = data.begin(); itD != data.end(); ++itD) {
    const ObsData_pl::iterator itU = findObs((*itD));
    if (itU != mData.end())
      *itU = *itD;
  }
}

void TimeBuffer::dropData(const SensorTime_v& dropped)
{
  for (SensorTime_v::const_iterator itS = dropped.begin(); itS != dropped.end(); ++itS) {
    const ObsData_pl::iterator itD = findObs(itS->time);
    if (itD != mData.end())
      mData.erase(itD);
  }
}

ObsData_pl::iterator TimeBuffer::findObs(ObsData_p obs)
{
  return std::lower_bound(mData.begin(), mData.end(), obs->sensorTime().time, ObsData_by_time());
}

ObsData_pl::iterator TimeBuffer::findObs(const Time& time)
{
  return std::lower_bound(mData.begin(), mData.end(), time, ObsData_by_time());
}

ObsData_pl::const_iterator TimeBuffer::findObs(const Time& time) const
{
  return std::lower_bound(mData.begin(), mData.end(), time, ObsData_by_time());
}
