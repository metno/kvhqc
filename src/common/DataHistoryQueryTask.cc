
#include "DataHistoryQueryTask.hh"

#include "TimeSpan.hh"
#include "sqlutil.hh"

namespace /*anonymous*/ {

Time my_qsql_time(const std::string& s)
{
  return timeutil::from_iso_extended_string(s);
}

bool initMetaType = false;

} // namespace anonymous

DataHistoryQueryTask::DataHistoryQueryTask(const SensorTime& st, size_t priority)
  : QueryTask(priority)
  , mSensorTime(st)
{
  if (not initMetaType) {
    qRegisterMetaType<SensorTime>("SensorTime");
    qRegisterMetaType<kvDataHistoryValues_v>("kvDataHistoryValues_v");
    initMetaType = true;
  }
}

QString DataHistoryQueryTask::querySql(QString dbversion) const
{
  std::ostringstream sql;
  sql << "SELECT dh.modificationtime, dh.corrected, dh.controlinfo, dh.useinfo, dh.cfailed"
      " FROM data_history dh WHERE ";
  sensor2sql(sql, mSensorTime.sensor, "dh.");
  sql << " AND dh.obstime = " << time2sql(mSensorTime.time)
      << " ORDER BY dh.version";
  return QString::fromStdString(sql.str());
}

void DataHistoryQueryTask::notifyRow(const ResultRow& row)
{
  kvDataHistoryValues v;
      
  int col = 0;
  v.modificationtime = my_qsql_time(row.getStdString(col++));
  v.corrected = row.getFloat(col++);;
  v.controlinfo = kvalobs::kvControlInfo(row.getStdString(col++));
  v.useinfo = kvalobs::kvUseInfo(row.getStdString(col++));
  v.cfailed = row.getStdString(col++);

  mHistory.push_back(v);
}

void DataHistoryQueryTask::notifyDone(const QString& withError)
{
  if (not withError.isNull())
    mHistory.clear();
  QueryTask::notifyDone(withError);
}
