
#ifndef ACCESS_TIMEBUFFER_HH
#define ACCESS_TIMEBUFFER_HH 1

#include "SimpleBuffer.hh"

class TimeBuffer : public SimpleBuffer
{
public:
  TimeBuffer(const Sensor& sensor, const TimeSpan& timeSpan, ObsFilter_p filter = ObsFilter_p());
  TimeBuffer(SignalRequest_p request);

  size_t size() const
    { return mData.size(); }

  ObsData_p get(const Time& time) const;
  
  virtual void newData(const ObsData_pv& data);
  virtual void updateData(const ObsData_pv& data);
  virtual void dropData(const SensorTime_v& dropped);

protected:
  const ObsData_pl& data() const
    { return mData; }

private:
  ObsData_pl::iterator findObs(ObsData_p obs);
  ObsData_pl::iterator findObs(const Time& st);
  ObsData_pl::const_iterator findObs(const Time& st) const;

private:
  ObsData_pl mData;
};

HQC_TYPEDEF_P(TimeBuffer);
HQC_TYPEDEF_PV(TimeBuffer);

#endif // ACCESS_TIMEBUFFER_HH
