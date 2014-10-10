
#include "StationQueryTask.hh"

#define MILOGGER_CATEGORY "kvhqc.StationQueryTask"
#include "common/ObsLogging.hh"

namespace /*anonymous*/ {

bool initMetaType = false;

} // namespace anonymous

StationQueryTask::StationQueryTask(size_t priority)
  : QueryTask(priority)
{
  METLIBS_LOG_SCOPE();
  if (not initMetaType) {
    qRegisterMetaType<hqc::kvStation_v>("kvStation_v");
    initMetaType = true;
  }
}

QString StationQueryTask::querySql(QString dbversion) const
{
  return "SELECT s.stationid, s.lat, s.lon, s.height, s.maxspeed, s.name, s.wmonr,"
      " s.nationalnr, s.icaoid, s.call_sign, s.stationstr, s.environmentid, s.static,"
      " s.fromtime FROM station s ORDER BY s.stationid";
}

void StationQueryTask::notifyRow(const ResultRow& row)
{
  METLIBS_LOG_SCOPE();
  int col = 0;

  const int stationid = row.getInt(col++);
  const float lat = row.getFloat(col++);
  const float lon = row.getFloat(col++);
  const float height = row.getFloat(col++);
  const float maxspeed = row.getFloat(col++);
  const std::string name = row.getStdString(col++);
  const int wmonr = row.getInt(col++);
  const int nationalnr = row.getInt(col++);
  const std::string icaoid = row.getStdString(col++);
  const std::string call_sign = row.getStdString(col++);
  const std::string stationstr = row.getStdString(col++);
  const int environmentid = row.getInt(col++);
  const bool is_static = row.getInt(col++);
  const timeutil::ptime fromtime = timeutil::from_iso_extended_string(row.getStdString(col++));

  mStations.push_back(kvalobs::kvStation(stationid, lat, lon, height, maxspeed,
          name, wmonr, nationalnr, icaoid, call_sign, stationstr, environmentid,
          is_static, fromtime));
}
