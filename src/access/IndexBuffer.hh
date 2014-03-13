
#ifndef ACCESS_INDEXBUFFER_HH
#define ACCESS_INDEXBUFFER_HH 1

#include "SimpleBuffer.hh"

class IndexBuffer : public SimpleBuffer
{
public:
  IndexBuffer(int stepSeconds, const Sensor& sensor, const TimeSpan& timeSpan, ObsFilter_p filter = ObsFilter_p());
  IndexBuffer(int stepSeconds, SignalRequest_p request);

  size_t size() const
    { return mData.size(); }

  ObsData_p get(int idx) const
    { return mData.at(idx); }
  
public:
  virtual void newData(const ObsData_pv& data);
  virtual void updateData(const ObsData_pv& data);
  virtual void dropData(const SensorTime_v& dropped);

private:
  int findIndex(ObsData_p obs) const;
  int findIndex(const Time& time) const;

private:
  int mStepSeconds;
  ObsData_pv mData;
};

HQC_TYPEDEF_P(IndexBuffer);
HQC_TYPEDEF_PV(IndexBuffer);

#endif // ACCESS_INDEXBUFFER_HH
