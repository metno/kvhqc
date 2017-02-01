
#include "SortedBuffer.hh"

#include "TimeBuffer.hh"

#define MILOGGER_CATEGORY "kvhqc.SortedBuffer"
#include "common/ObsLogging.hh"

namespace {
struct ObsData_eq_ST : public std::binary_function<SensorTime, ObsData_p, bool> {
  bool operator()(const SensorTime& a, ObsData_p b) const
    { return eq_SensorTime()(a, b->sensorTime()); }
};
}

SortedBuffer::SortedBuffer(Ordering_p ordering, const Sensor_s& sensors, const TimeSpan& timeSpan, ObsFilter_p filter)
  : SimpleBuffer(sensors, timeSpan, filter)
  , mOrdering(ordering)
{
}

SortedBuffer::SortedBuffer(Ordering_p ordering, SignalRequest_p request)
  : SimpleBuffer(request)
  , mOrdering(ordering)
{
}

void SortedBuffer::setOrdering(Ordering_p o)
{
  mOrdering = o;
  sort();
}

Time_s SortedBuffer::times() const
{
  Time_s times;
  for (ObsData_pv::const_iterator itD = mData.begin(); itD != mData.end(); ++itD)
    times.insert((*itD)->sensorTime().time);
  return times;
}

ObsData_p SortedBuffer::get(const SensorTime& st) const
{
  const ObsData_pv::const_iterator it = findSorted(st);
  if (it != mData.end())
    return *it;
  return ObsData_p();
}

void SortedBuffer::onNewData(const ObsData_pv& data)
{
  METLIBS_LOG_SCOPE();
  bool inserted = false;
  for (ObsData_pv::const_iterator itD = data.begin(); itD != data.end(); ++itD) {
    if (not *itD) {
      HQC_LOG_ERROR("null data in newData");
      continue;
    }
    const ObsData_pv::iterator it = findUnsorted((*itD)->sensorTime());
    if (it != mData.end()) {
      HQC_LOG_WARN("replacing data in newData" << (*itD)->sensorTime());
      *it = *itD; // FIXME the need for this is actually a bug in some ObsAccess implementation
    } else {
      mData.push_back(*itD);
      inserted = true;
    }
  }
  if (inserted)
    sort();
}

void SortedBuffer::onUpdateData(const ObsData_pv& data)
{
  METLIBS_LOG_SCOPE();
  for (ObsData_pv::const_iterator itD = data.begin(); itD != data.end(); ++itD) {
    ObsData_p du = *itD;
    METLIBS_LOG_DEBUG(LOGVAL(du->sensorTime()) << LOGVAL(du->corrected()));
    const ObsData_pv::iterator it = findUnsorted(du->sensorTime());
    if (it != mData.end())
      *it = du;
    else
      mData.push_back(*itD);
  }
  sort();
}

void SortedBuffer::onDropData(const SensorTime_v& dropped)
{
  // drop cannot change ordering
  METLIBS_LOG_SCOPE();
  for (SensorTime_v::const_iterator itS = dropped.begin(); itS != dropped.end(); ++itS) {
    const ObsData_pv::iterator it = findSorted(*itS);
    if (it != mData.end())
      mData.erase(it);
  }
}

void SortedBuffer::sort()
{
  METLIBS_LOG_SCOPE();
  std::sort(mData.begin(), mData.end(), OrderingHelper(mOrdering));
}

ObsData_pv::const_iterator SortedBuffer::findSorted(const SensorTime& st) const
{
  const ObsData_pv::const_iterator it = std::lower_bound(mData.begin(), mData.end(), st, OrderingHelper(mOrdering));
  if (it != mData.end() and eq_SensorTime()(st, (*it)->sensorTime()))
    return it;
  return mData.end();
}

ObsData_pv::iterator SortedBuffer::findSorted(const SensorTime& st)
{
  const ObsData_pv::iterator it = std::lower_bound(mData.begin(), mData.end(), st, OrderingHelper(mOrdering));
  if (it != mData.end() and eq_SensorTime()(st, (*it)->sensorTime()))
    return it;
  return mData.end();
}

ObsData_pv::iterator SortedBuffer::findUnsorted(const SensorTime& st)
{
  return std::find_if(mData.begin(), mData.end(), std::bind1st(ObsData_eq_ST(), st));
}
