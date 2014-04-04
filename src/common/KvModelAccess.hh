
#ifndef KvModelAccess_hh
#define KvModelAccess_hh 1

#include "KvalobsModelData.hh"
#include "ModelAccess.hh"

class KvModelAccess : public ModelAccess {
public:
  virtual ModelDataPtr find(const SensorTime& st);
  virtual ModelDataSet findMany(const std::vector<SensorTime>& sensorTimes);
  virtual ModelDataSet allData(const std::vector<Sensor>& sensors, const TimeSpan& limits);

protected:
  KvalobsModelDataPtr receive(const kvalobs::kvModelData& data);
  bool drop(const SensorTime& st);

protected:
  typedef std::map<SensorTime, KvalobsModelDataPtr, lt_ModelSensorTime> Data_t;
  Data_t mData;
};
typedef boost::shared_ptr<KvModelAccess> KvModelAccessPtr;

#endif // KvModelAccess_hh
