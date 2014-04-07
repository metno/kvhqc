
#ifndef ACCESS_TIMEBUFFER_HH
#define ACCESS_TIMEBUFFER_HH 1

#include "SimpleBuffer.hh"

class TimeBuffer : public SimpleBuffer
{
public:
  struct ObsData_by_time {
    bool operator()(ObsData_p a, ObsData_p b) const
      { return lt_SensorTime()(a->sensorTime(), b->sensorTime()); }
    bool operator()(ObsData_p a, const SensorTime& b) const
      { return lt_SensorTime()(a->sensorTime(), b); }
    bool operator()(const SensorTime& a, ObsData_p b) const
      { return lt_SensorTime()(a, b->sensorTime()); }
  };

  typedef std::set<ObsData_p, ObsData_by_time> ObsDataByTime_ps;

public:
  TimeBuffer(const Sensor_s& sensors, const TimeSpan& timeSpan, ObsFilter_p filter = ObsFilter_p());
  TimeBuffer(SignalRequest_p request);

  size_t size() const
    { return mData.size(); }

  Time_s times() const;

  ObsData_p get(const SensorTime& st) const;
  
  virtual void onNewData(const ObsData_pv& data);
  virtual void onUpdateData(const ObsData_pv& data);
  virtual void onDropData(const SensorTime_v& dropped);

  const ObsDataByTime_ps& data() const
    { return mData; }

private:
  ObsDataByTime_ps::iterator find(const SensorTime& st);
  ObsDataByTime_ps::const_iterator find(const SensorTime& st) const;

private:
  ObsDataByTime_ps mData;
};

HQC_TYPEDEF_P(TimeBuffer);
HQC_TYPEDEF_PV(TimeBuffer);

#endif // ACCESS_TIMEBUFFER_HH
