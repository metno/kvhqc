
#include "FindExtremeValues.hh"

#include "KvHelpers.hh"
#include "StationInfoBuffer.hh"
#include "common/HqcApplication.hh"

#include <boost/foreach.hpp>

#include <sstream>

#define MILOGGER_CATEGORY "kvhqc.FindExtremeValues"
#include "common/ObsLogging.hh"

namespace /*anonymous*/ {
std::string exists_in_obspgm(const std::string& data_alias, const std::string& obs_pgm_alias = "o")
{
  const std::string &d = data_alias, &o = obs_pgm_alias;
  std::ostringstream out;
  out << "EXISTS (SELECT * FROM obs_pgm AS " << o << " WHERE " << o << ".stationid = " << d << ".stationid"
      " AND " << o << ".paramid = " << d << ".paramid"
      " AND " << o << ".typeid  = " << d << ".typeid"
      " AND " << o << ".fromtime <= " << d << ".obstime"
      " AND (" << o << ".totime IS NULL OR " << o << ".totime >= " << d << ".obstime))";
  return out.str();
}
} // namespace anonymous

namespace Extremes {

std::vector<SensorTime> find(int paramid, const TimeRange& tLimits)
{
  METLIBS_LOG_SCOPE();

  const listStat_l& stationDetails = StationInfoBuffer::instance()->getStationDetails();
  std::ostringstream excluded_station_list;
  const char* sep = "";
  BOOST_FOREACH(const listStat_t& ls, stationDetails) {
    if (Helpers::isNorwegianStationId(ls.stationid)
        and (ls.municipid <= 100 or ls.municipid >= 2100))
    {
      excluded_station_list << sep << ls.stationid;
      sep = ",";
    }
  }
  const std::string excludedIds = excluded_station_list.str();
  METLIBS_LOG_DEBUG(LOGVAL(excludedIds));

  std::ostringstream paramids;
  bool findMaximum = true;
  if (paramid == kvalobs::PARAMID_TAN) {
    // TA, TAN, TAN_12
    paramids << "211,213,214";
    findMaximum = false;
  } else if (paramid == kvalobs::PARAMID_TAX) {
    // TA, TAX, TAX_12
    paramids << "211,215,216";
  } else if (paramid == kvalobs::PARAMID_FG) {
    // FG, FG_010, FG_1, FG_6, FG_12, FG_X
    paramids << "83,84,90,91,92,94";
  } else if (paramid == kvalobs::PARAMID_FX) {
    // FX, FX_1, FX_6, FX_12, FX_X, FX_3
    paramids << "86,87,88,89,93,95";
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
  sql << "SELECT " << hqcApp->kvalobsColumnsSensorTime("d") << " FROM data AS d,"
      "  (SELECT dd.stationid AS s, " << function << "(dd.corrected) AS c FROM data AS dd"
      "   WHERE " << Helpers::isNorwegianStationIdSQL("dd.stationid");
  if (not excludedIds.empty())
    sql << " AND dd.stationid NOT IN (" << excludedIds << ")";
  sql << "   AND dd.paramid IN (" << paramids.str() << ")"
      "   AND " << exists_in_obspgm("dd") <<
      "   AND (substr(dd.useinfo,3,1) IN ('0','1','2')"
      "        OR (substr(dd.useinfo,3,1) = '3' AND dd.original = dd.corrected))"
      "   AND " << c_obstime.str();

  // exclude some special values (no measurement, no snow, no rain, ...)
  if (paramid == kvalobs::PARAMID_RR_24)
    sql << "   AND dd.corrected != -1";
  else if (paramid == kvalobs::PARAMID_SA)
    sql << "   AND dd.corrected != -3 AND dd.corrected != -1 AND dd.corrected != 0";

  sql << " GROUP BY s ORDER BY c " << ordering << " LIMIT " << n_extremes << ") AS ex"
      " WHERE stationid = ex.s AND corrected = ex.c AND paramid IN (" << paramids.str() << ") AND " << c_obstime.str() <<
      "   AND " << exists_in_obspgm("d") <<
      " ORDER BY corrected " << ordering << ", stationid, obstime";
  METLIBS_LOG_DEBUG(LOGVAL(sql.str()));

  return hqcApp->kvalobsQuerySensorTime(sql.str());
}

} // namespace Extremes
