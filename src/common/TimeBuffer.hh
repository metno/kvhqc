
#ifndef ACCESS_TIMEBUFFER_HH
#define ACCESS_TIMEBUFFER_HH 1

#include "ObsDataSet.hh"
#include "SimpleBuffer.hh"

class TimeBuffer : public SimpleBuffer
{
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

  const ObsData_ps_ST& data() const
    { return mData; }

private:
  ObsData_ps_ST::iterator find(const SensorTime& st);
  ObsData_ps_ST::const_iterator find(const SensorTime& st) const;

private:
  ObsData_ps_ST mData;
};

HQC_TYPEDEF_P(TimeBuffer);
HQC_TYPEDEF_PV(TimeBuffer);

#endif // ACCESS_TIMEBUFFER_HH
