
#ifndef ACCESS_SORTEDBUFFER_HH
#define ACCESS_SORTEDBUFFER_HH 1

#include "ObsDataSet.hh"
#include "SimpleBuffer.hh"

class SortedBuffer : public SimpleBuffer
{
public:
  class Ordering {
  public:
    virtual ~Ordering() { }
    virtual bool compare(ObsData_p a, ObsData_p b) const
      { return compare(a->sensorTime(), b->sensorTime()); }
    virtual bool compare(const SensorTime& a, const SensorTime& b) const = 0;
  };
  typedef boost::shared_ptr<Ordering> Ordering_p;

  struct OrderingHelper {
    OrderingHelper(Ordering_p o)
      : ordering(o) { }
    
    bool operator()(ObsData_p a, ObsData_p b) const
      { return ordering->compare(a, b); }

    bool operator()(ObsData_p a, const SensorTime& b) const
      { return ordering->compare(a->sensorTime(), b); }
    bool operator()(const SensorTime& a, ObsData_p b) const
      { return ordering->compare(a, b->sensorTime()); }
    bool operator()(const SensorTime& a, const SensorTime& b) const
      { return ordering->compare(a, b); }
    
    Ordering_p ordering;
  };

  SortedBuffer(Ordering_p ordering, const Sensor_s& sensors, const TimeSpan& timeSpan, ObsFilter_p filter = ObsFilter_p());
  SortedBuffer(Ordering_p ordering, SignalRequest_p request);

  void setOrdering(Ordering_p o);
  Ordering_p ordering() const
    { return mOrdering; }

  size_t size() const
    { return mData.size(); }

  Time_s times() const;

  ObsData_p get(const SensorTime& st) const;
  
  virtual void onNewData(const ObsData_pv& data);
  virtual void onUpdateData(const ObsData_pv& data);
  virtual void onDropData(const SensorTime_v& dropped);

  const ObsData_pv& data() const
    { return mData; }

private:
  ObsData_pv::const_iterator findSorted(const SensorTime& st) const;
  ObsData_pv::iterator findSorted(const SensorTime& st);
  ObsData_pv::iterator findUnsorted(const SensorTime& st);
  void sort();

private:
  Ordering_p mOrdering;
  ObsData_pv mData;
};

HQC_TYPEDEF_P(SortedBuffer);
HQC_TYPEDEF_PV(SortedBuffer);

#endif // ACCESS_SORTEDBUFFER_HH
