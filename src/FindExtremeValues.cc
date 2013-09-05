
#include "FindExtremeValues.hh"

#include "Helpers.hh"
#include "HqcApplication.hh"
#include "StationInfoBuffer.hh"

#include <QtCore/QVariant>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

#include <boost/foreach.hpp>

#include <sstream>

#define MILOGGER_CATEGORY "kvhqc.FindExtremeValues"
#include "HqcLogging.hh"

namespace Extremes {

std::vector<SensorTime> find(int paramid, const TimeRange& tLimits)
{
  METLIBS_LOG_SCOPE();
  const listStat_l& stationDetails = StationInfoBuffer::instance()->getStationDetails();
  std::ostringstream excluded_station_list;
  const char* sep = "";
  BOOST_FOREACH(const listStat_t& ls, stationDetails) {
    if (ls.stationid >= 60 and ls.stationid < 100000
        and (ls.municipid <= 100 or ls.municipid >= 2100))
    {
      excluded_station_list << sep << ls.stationid;
      sep = ",";
    }
  }
  METLIBS_LOG_DEBUG(LOGVAL(excluded_station_list.str()));

  std::ostringstream paramids;
  bool findMaximum = true;
  if (paramid == kvalobs::PARAMID_TAN or paramid == kvalobs::PARAMID_TAX) {
    paramids << kvalobs::PARAMID_TA << ',' << paramid;
    findMaximum = (paramid == kvalobs::PARAMID_TAX);
  } else {
    paramids << paramid;
  }
  const std::string function = findMaximum ? "MAX"  : "MIN";
  const std::string ordering = findMaximum ? "DESC" : "ASC";

  const int n_extremes = 20;

  std::ostringstream c_obstime;
  c_obstime << "obstime > '" << timeutil::to_iso_extended_string(tLimits.t0())
            << "' AND obstime <= '" << timeutil::to_iso_extended_string(tLimits.t1()) << "'";

  std::ostringstream sql;
  sql << "SELECT stationid,paramid,level,sensor,typeid,obstime,original,corrected,controlinfo,cfailed FROM data,"
      "  (SELECT stationid AS s, " << function << "(corrected) AS c FROM data"
      "   WHERE stationid BETWEEN 60 AND 100000 AND stationid NOT IN (" << excluded_station_list.str() << ")"
      "   AND paramid IN (" << paramids.str() << ")"
      "   AND (substr(useinfo,3,1) IN ('0','1','2')"
      "        OR substr(useinfo,3,1) = '3' AND original = corrected)"
      "   AND " << c_obstime.str() <<
      " GROUP BY s ORDER BY c " << ordering << " LIMIT " << n_extremes << ") AS ex"
      " WHERE stationid = ex.s AND corrected = ex.c AND paramid IN (" << paramids.str() << ") AND " << c_obstime.str() <<
      " ORDER BY corrected " << ordering << ", stationid, obstime";
  METLIBS_LOG_DEBUG(LOGVAL(sql.str()));

  QSqlQuery query(hqcApp->kvalobsDB());
  std::vector<SensorTime> extremes;
  if (query.exec(QString::fromStdString(sql.str()))) {
    while (query.next()) {
      const Sensor s(query.value(0).toInt(), query.value(1).toInt(), query.value(2).toInt(),
          query.value(3).toInt(), query.value(4).toInt());
      const timeutil::ptime t = timeutil::from_iso_extended_string(query.value(5).toString().toStdString());
      const SensorTime st(s, t);
      extremes.push_back(st);
      METLIBS_LOG_DEBUG(LOGVAL(st));
    }
  } else {
    METLIBS_LOG_ERROR("search for extreme values failed: " << query.lastError().text());
  }
  return extremes;
}

} // namespace Extremes
