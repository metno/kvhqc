
#include "DataQueryTask.hh"

#include "KvalobsData.hh"
#include "KvHelpers.hh"
#include "ObsDataSet.hh"
#include "sqlutil.hh"

#include <boost/make_shared.hpp>

namespace /*anonymous*/ {

Time my_qsql_time(const std::string& s)
{
  return timeutil::from_iso_extended_string(s);
}

const size_t QUERY_DATA_CHUNKSIZE = 32;

} // namespace anonymous

DataQueryTask::DataQueryTask(ObsRequest_p request, size_t priority)
  : QueryTask(priority)
  , mRequest(request)
{
}

QString DataQueryTask::querySql(QString dbversion) const
{
  const Sensor_s& sensors = mRequest->sensors();
  const TimeSpan& time = mRequest->timeSpan();
  ObsFilter_p filter = mRequest->filter();

  std::ostringstream sql;
  sql << "SELECT d.stationid, d.paramid, d.typeid, d.level, d.sensor,"
      " d.obstime, d.original, d.tbtime, d.corrected, d.controlinfo, d.useinfo, d.cfailed"
      " FROM data d WHERE ";
  sensors2sql(sql, sensors, "d.");
  sql << " AND d.obstime BETWEEN " << time2sql(time.t0()) << " AND " << time2sql(time.t1());
  if (filter and filter->hasSQL())
    sql << " AND (" << filter->acceptingSQL("d.") << ")";
  //sql << " ORDER BY d.stationid, d.paramid, d.typeid, d.level, d.sensor, d.obstime";
  return QString::fromStdString(sql.str());
}

void DataQueryTask::notifyRow(const ResultRow& row)
{
  int col = 0;
  
  const int stationid = row.getInt(col++);
  const int paramid   = row.getInt(col++);
  const int type_id   = row.getInt(col++);
  const int level     = row.getInt(col++);
  const int sensornr  = row.getInt(col++);
      
  const Time  obstime   = my_qsql_time(row.getStdString(col++));
  const float original  = row.getFloat(col++);
  const Time  tbtime    = my_qsql_time(row.getStdString(col++));
  const float corrected = row.getFloat(col++);;
  const kvalobs::kvControlInfo controlinfo(row.getStdString(col++));
  const kvalobs::kvUseInfo     useinfo    (row.getStdString(col++));
  const std::string cfailed = row.getStdString(col++);
  
  const kvalobs::kvData kvdata(stationid, obstime, original, paramid,
      tbtime, type_id, sensornr, level, corrected, controlinfo, useinfo, cfailed);
  KvalobsData_p kd = boost::make_shared<KvalobsData>(kvdata, false);
  ObsFilter_p filter = mRequest->filter();
  if ((not filter) or filter->accept(kd, true)) {
    mData.push_back(kd);
    if (mData.size() >= QUERY_DATA_CHUNKSIZE)
      sendData();
  }
}

void DataQueryTask::sendData()
{
  std::sort(mData.begin(), mData.end(), ObsData_by_SensorTime());

  Q_EMIT newData(mRequest, mData);
  mData.clear();
}

void DataQueryTask::notifyStatus(int status)
{
  if (status >= COMPLETE) {
    if (not mData.empty())
      sendData();
    //Q_EMIT newData(mRequest, mData); // empty data
  }
  Q_EMIT queryStatus(mRequest, status);
}

void DataQueryTask::notifyError(QString)
{
  Q_EMIT queryStatus(mRequest, QueryTask::FAILED);
}
