
#include "StationParamSQLTask.hh"

#include "TimeSpan.hh"

namespace /*anonymous*/ {

Time my_qsql_time(const std::string& s)
{
  return timeutil::from_iso_extended_string(s);
}

bool initMetaType = false;

} // namespace anonymous

StationParamSQLTask::StationParamSQLTask(const SensorTime& st, size_t priority)
  : QueryTask(priority)
  , mSensorTime(st)
{
  if (not initMetaType) {
    qRegisterMetaType<SensorTime>("SensorTime");
    initMetaType = true;
  }
}

QString StationParamSQLTask::querySql(QString /*dbversion*/) const
{
  const int day = mSensorTime.time.date().day_of_year();

  const Sensor& s = mSensorTime.sensor;

  return QString("SELECT sp.metadata, sp.fromtime"
      " FROM station_param sp")
      + QString(" WHERE sp.stationid = %1").arg(s.stationId)
      + QString("   AND sp.paramid = %2").arg(s.paramId)
      + QString("   AND sp.level = %3").arg(s.level)
      + QString("   AND sp.sensor = '%4'").arg(s.sensor)
      + QString("   AND %5 BETWEEN sp.fromday AND sp.today").arg(day)
      + QString("   AND sp.hour = -1");
}

void StationParamSQLTask::notifyRow(const ResultRow& row)
{
  mMetadata.push_back(row.getStdString(0));
  mFromtimes.push_back(my_qsql_time(row.getStdString(1)));
}

void StationParamSQLTask::notifyDone(const QString& withError)
{
  if (not withError.isNull())
    mMetadata.clear();
  QueryTask::notifyDone(withError);
}
