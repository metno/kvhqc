
#ifndef COMMON_STATIONPARAMSQLTASK_HH
#define COMMON_STATIONPARAMSQLTASK_HH 1

#include "KvTypedefs.hh"
#include "QueryTask.hh"
#include "Sensor.hh"

class StationParamSQLTask : public QueryTask
{
public:
  StationParamSQLTask(const SensorTime& st, size_t priority);
  
  QString querySql(QString dbversion) const;
  void notifyRow(const ResultRow& row);
  void notifyDone(const QString& withError);
  
  const SensorTime& sensorTime() const
    { return mSensorTime; }

  const hqc::string_v& metadata() const
    { return mMetadata; }

  typedef std::vector<timeutil::ptime> time_v;

  const time_v& fromtimes() const
    { return mFromtimes; }

private:
  SensorTime mSensorTime;
  hqc::string_v mMetadata;
  time_v mFromtimes;
};

#endif // COMMON_STATIONPARAMSQLTASK_HH
