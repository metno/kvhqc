
#ifndef ACCESS_SINGLEOBSBUFFER_HH
#define ACCESS_SINGLEOBSBUFFER_HH 1

#include "SimpleBuffer.hh"

class SingleObsBuffer : public SimpleBuffer
{
public:
  SingleObsBuffer(const SensorTime& sensorTime);

  ObsData_p get() const
    { return mObs; }
  
  virtual void onNewData(const ObsData_pv& data) override;
  virtual void onUpdateData(const ObsData_pv& data) override;
  virtual void onDropData(const SensorTime_v& dropped) override;

private:
  bool match(const SensorTime& st) const;

private:
  ObsData_p mObs;
};

HQC_TYPEDEF_P(SingleObsBuffer);

#endif // ACCESS_SINGLEOBSBUFFER_HH
