
#ifndef ACCESS_INDEXBUFFER_HH
#define ACCESS_INDEXBUFFER_HH 1

#include "SimpleBuffer.hh"

class IndexBuffer : public SimpleBuffer
{
public:
  IndexBuffer(int stepSeconds, const Sensor_s& sensors, const TimeSpan& timeSpan, ObsFilter_p filter = ObsFilter_p());
  IndexBuffer(int stepSeconds, SignalRequest_p request);

  size_t size() const
    { return mSize; }

  ObsData_p get(const Sensor& sensor, int idx) const;
  
public:
  virtual void newData(const ObsData_pv& data);
  virtual void updateData(const ObsData_pv& data);
  virtual void dropData(const SensorTime_v& dropped);

private:
  int findIndex(ObsData_p obs) const;
  int findIndex(const SensorTime& st) const;

private:
  int mStepSeconds;
  int mSize;

  typedef std::map<Sensor, ObsData_pv, lt_Sensor> data_m;
  data_m mData;
};

HQC_TYPEDEF_P(IndexBuffer);
HQC_TYPEDEF_PV(IndexBuffer);

#endif // ACCESS_INDEXBUFFER_HH
