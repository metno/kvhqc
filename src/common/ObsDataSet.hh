
#ifndef COMMON_OBSDATASET_HH
#define COMMON_OBSDATASET_HH 1

#include "ObsData.hh"
#include <functional>

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

struct ObsData_by_Corrected {
  ObsData_by_Corrected(bool a) : ascending(a) { }
  bool operator()(ObsData_p a, ObsData_p b) const
    { return compare(a->corrected(), b->corrected()); }
  bool operator()(ObsData_p a, float b) const
    { return compare(a->corrected(), b); }
  bool operator()(const float& a, ObsData_p b) const
    { return compare(a, b->corrected()); }
private:
  bool compare(float a, float b) const
    { return ascending ? (a < b) : (a > b); }
  bool ascending;
};

typedef std::set<ObsData_p, ObsData_by_SensorTime> ObsData_ps_ST;

#endif // COMMON_OBSDATASET_HH
