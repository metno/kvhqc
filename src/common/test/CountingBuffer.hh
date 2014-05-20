
#ifndef COUNTINGBUFFER_HH
#define COUNTINGBUFFER_HH 1

#include "TimeBuffer.hh"
#include "util/make_set.hh"

class CountingBuffer : public TimeBuffer
{
public:
  CountingBuffer(const Sensor_s& sensors, const TimeSpan& timeSpan, ObsFilter_p filter = ObsFilter_p())
    : TimeBuffer(sensors, timeSpan, filter) { reset(); }

  CountingBuffer(const Sensor& sensor, const TimeSpan& timeSpan, ObsFilter_p filter = ObsFilter_p())
    : TimeBuffer(make_set<Sensor_s>(sensor), timeSpan, filter) { reset(); }

  void reset()
    { countComplete = countNew = countUpdate = countDrop =0; }

  virtual void completed(bool failed)
    { TimeBuffer::completed(failed); countComplete += 1; }

  virtual void onNewData(const ObsData_pv& data)
    { TimeBuffer::onNewData(data); countNew += 1; }

  virtual void onUpdateData(const ObsData_pv& data)
    { TimeBuffer::onUpdateData(data); countUpdate += 1; }

  virtual void onDropData(const SensorTime_v& drop)
    { TimeBuffer::onDropData(drop); countDrop += 1; }

  size_t countComplete, countNew, countUpdate, countDrop;
};

HQC_TYPEDEF_P(CountingBuffer);

#endif // COUNTINGBUFFER_HH