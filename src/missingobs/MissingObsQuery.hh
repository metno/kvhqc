
#ifndef MISSINGOBSQUERY_HH
#define MISSINGOBSQUERY_HH 1

#include "common/KvTypedefs.hh"
#include "common/QueryTask.hh"
#include "common/Sensor.hh"
#include "common/TimeSpan.hh"

class MissingObsQuery : public QueryTask
{
public:
  MissingObsQuery(const TimeSpan& time, const hqc::int_s& typeids, size_t priority);
  
  QString querySql(QString dbversion) const;

  void notifyRow(const ResultRow& row);
  void notifyDone(const QString& withError);

  const TimeSpan& time() const
    { return mTime; }

  const SensorTime_v& missing() const
    { return mMissing; }

private:
  TimeSpan mTime;
  hqc::int_s mTypeIds;
  SensorTime_v mMissing;
};

#endif // MISSINGOBSQUERY_HH
