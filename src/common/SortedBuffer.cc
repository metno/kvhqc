/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2014-2018 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: kvalobs-dev@met.no

  This file is part of HQC

  HQC is free software; you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  HQC is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General Public License
  along with HQC; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/

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
  for (ObsData_p obs : mData)
    times.insert(obs->sensorTime().time);
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
  for (ObsData_p o : data) {
    if (!o) {
      HQC_LOG_ERROR("null data in newData");
      continue;
    }
    const ObsData_pv::iterator it = findUnsorted(o->sensorTime());
    if (it != mData.end()) {
      ObsData_p& e = *it;
      if (o->original() != e->original() || o->corrected() != e->corrected() || o->controlinfo() != e->controlinfo() || o->cfailed() != e->cfailed()) {
        HQC_LOG_WARN("replacing data in newData" << o->sensorTime() << "; new/existing original=" << o->original() << '/' << e->original()
                                                 << " corrected=" << o->corrected() << '/' << e->corrected() << " controlinfo=" << o->controlinfo().flagstring()
                                                 << '/' << e->controlinfo().flagstring() << " cfailed=" << o->cfailed() << '/' << e->cfailed());
      }
      e = o; // FIXME the need for this is actually a bug in some ObsAccess implementation
    } else {
      mData.push_back(o);
      inserted = true;
    }
  }
  if (inserted)
    sort();
}

void SortedBuffer::onUpdateData(const ObsData_pv& data)
{
  METLIBS_LOG_SCOPE();
  for (ObsData_p du : data) {
    METLIBS_LOG_DEBUG(LOGVAL(du->sensorTime()) << LOGVAL(du->corrected()));
    const ObsData_pv::iterator it = findUnsorted(du->sensorTime());
    if (it != mData.end())
      *it = du;
    else
      mData.push_back(du);
  }
  sort();
}

void SortedBuffer::onDropData(const SensorTime_v& dropped)
{
  // drop cannot change ordering
  METLIBS_LOG_SCOPE();
  for (const SensorTime& st : dropped) {
    const ObsData_pv::iterator it = findSorted(st);
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
