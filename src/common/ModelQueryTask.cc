
#include "ModelQueryTask.hh"

#include "ModelData.hh"
#include "sqlutil.hh"

#include <boost/make_shared.hpp>

#define MILOGGER_CATEGORY "kvhqc.ModelQueryTask"
#include "common/ObsLogging.hh"

namespace /*anonymous*/ {

inline timeutil::ptime my_qsql_time(const std::string& s)
{
  return timeutil::from_iso_extended_string(s);
}

bool initMetaType = false;
const size_t QUERY_DATA_CHUNKSIZE = 32;

} // namespace anonymous

ModelQueryTask::ModelQueryTask(const SensorTime_v& sensorTimes, size_t priority)
  : QueryTask(priority)
  , mSensorTimes(sensorTimes)
{
  METLIBS_LOG_SCOPE();
  if (not initMetaType) {
    qRegisterMetaType<ModelData_pv>("ModelData_pv");
    initMetaType = true;
  }
}

QString ModelQueryTask::querySql(QString dbversion) const
{
  std::ostringstream sql;
  sql << "SELECT m.stationid, m.paramid, m.level, m.obstime, m.original"
      " FROM model_data m WHERE ";
  sensortimes2sql(sql, mSensorTimes, "m.", true);
  return QString::fromStdString(sql.str());
}

void ModelQueryTask::notifyRow(const ResultRow& row)
{
  METLIBS_LOG_SCOPE();
  int col = 0;

  SensorTime st;
  st.sensor.stationId = row.getInt(col++);
  st.sensor.paramId   = row.getInt(col++);
  st.sensor.level     = row.getInt(col++);
  st.time = my_qsql_time(row.getStdString(col++));
  const float value   = row.getFloat(col++);

  METLIBS_LOG_DEBUG(LOGVAL(st) << LOGVAL(value));

  mData.push_back(boost::make_shared<ModelData>(st, value));
  if (mData.size() >= QUERY_DATA_CHUNKSIZE)
    sendData();
}

void ModelQueryTask::sendData()
{
  if (mData.empty())
    return;
  
  std::sort(mData.begin(), mData.end(), lt_ModelData_p());

  Q_EMIT data(mData);
  mData.clear();
}

void ModelQueryTask::notifyDone()
{
  if (not mData.empty())
    sendData();
  Q_EMIT completed(false);
}

void ModelQueryTask::notifyError(QString message)
{
  HQC_LOG_ERROR(message);
  Q_EMIT completed(true);
}
