
#ifndef COMMON_OBSDATASET_HH
#define COMMON_OBSDATASET_HH 1

#include "ObsData.hh"

struct ObsData_by_SensorTime {
  bool operator()(ObsData_p a, ObsData_p b) const
    { return lt_SensorTime()(a->sensorTime(), b->sensorTime()); }
  bool operator()(ObsData_p a, const SensorTime& b) const
    { return lt_SensorTime()(a->sensorTime(), b); }
  bool operator()(const SensorTime& a, ObsData_p b) const
    { return lt_SensorTime()(a, b->sensorTime()); }
};

struct ObsData_by_Time {
  bool operator()(ObsData_p a, ObsData_p b) const
    { return a->sensorTime().time < b->sensorTime().time; }
  bool operator()(ObsData_p a, const timeutil::ptime& b) const
    { return a->sensorTime().time < b; }
  bool operator()(const timeutil::ptime& a, ObsData_p b) const
    { return a < b->sensorTime().time; }
};

typedef std::set<ObsData_p, ObsData_by_SensorTime> ObsData_ps_ST;

#endif // COMMON_OBSDATASET_HH
