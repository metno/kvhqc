
#include "StationParamSQLTask.hh"

#include "TimeSpan.hh"

#define MILOGGER_CATEGORY "kvhqc.StationParamSQLTask"
#include "common/ObsLogging.hh"

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
  QString sql("SELECT sp.metadata, sp.fromtime"
              " FROM station_param sp");
  if (mSensorTime.valid()) {
    const int day = mSensorTime.time.date().day_of_year();
    const Sensor& s = mSensorTime.sensor;
    sql += QString(" WHERE sp.stationid = %1").arg(s.stationId)
         + QString("   AND sp.paramid = %2").arg(s.paramId)
         + QString("   AND sp.level = %3").arg(s.level)
         + QString("   AND sp.sensor = '%4'").arg(s.sensor)
         + QString("   AND %5 BETWEEN sp.fromday AND sp.today").arg(day)
         + QString("   AND sp.hour = -1");
  } else {
    METLIBS_LOG_ERROR("requesting station param for invalid sensortime " << mSensorTime);
    sql += QString(" WHERE 0 = 1");
  }
  return sql;
}

void StationParamSQLTask::notifyRow(const ResultRow& row)
{
  std::string metadata, fromtimetext;
  try {
    metadata = row.getStdString(0);
    fromtimetext = row.getStdString(1);

    mMetadata.push_back(metadata);
    mFromtimes.push_back(my_qsql_time(fromtimetext));
  } catch (std::exception& ex) {
    METLIBS_LOG_ERROR("exception while reading station param for " << mSensorTime << " row[0]='" << metadata << "' row[1]='" << fromtimetext
                                                                   << "' ex=" << ex.what());
  }
}

void StationParamSQLTask::notifyDone(const QString& withError)
{
  if (not withError.isNull())
    mMetadata.clear();
  QueryTask::notifyDone(withError);
}
