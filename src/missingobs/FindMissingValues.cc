
#include "FindMissingValues.hh"

#include "KvHelpers.hh"
#include "StationInfoBuffer.hh"
#include "common/HqcApplication.hh"

#include <QtCore/QVariant>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

#include <boost/foreach.hpp>

#include <sstream>

#define M_TIME
#define MILOGGER_CATEGORY "kvhqc.FindMissingValues"
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

namespace Missing {

std::vector<SensorTime> find(const std::vector<int>& typeIds, const TimeSpan& tLimits)
{
  METLIBS_LOG_TIME();
  METLIBS_LOG_DEBUG(LOGVAL(tLimits));

  std::ostringstream sql;
  sql << "SELECT o.stationid, o.typeid, t.s"
      <<  " FROM obs_pgm AS o,"
      << "     (values ";

  const timeutil::pdate d0 = tLimits.t0().date(), d1 = tLimits.t1().date();
  for (timeutil::pdate d = d0; d <= d1; d += boost::gregorian::days(1)) {
    const timeutil::ptime t6(d, boost::posix_time::hours(6)), t7(d, boost::posix_time::hours(7));
    METLIBS_LOG_DEBUG(LOGVAL(t6) << LOGVAL(t7));
    sql << "(timestamp '" << timeutil::to_iso_extended_string(t6)
        << "'), (timestamp '" << timeutil::to_iso_extended_string(t7) << "')";
    if (d < d1)
      sql << ',';
  }
  sql << ") as t(s)"
      << " WHERE " << Helpers::isNorwegianStationIdSQL("o.stationid");
  if (typeIds.size() == 1) {
    sql << "   AND o.typeid = " << typeIds.front();
  } else if (typeIds.size() > 1) {
    sql << "   AND o.typeid IN ";
    char sep = '(';
    BOOST_FOREACH(int t, typeIds) {
      sql << sep << t;
      sep = ',';
    }
    sql << ")";
  }
  sql << "   AND o.paramid = 110"
      << "   AND o.fromtime <= t.s AND (o.totime IS NULL OR o.totime >= t.s)"
      << "   AND    ((extract(hour from t.s) =  6 AND o.kl06 = TRUE)"
      << "        OR (extract(hour from t.s) =  7 AND o.kl07 = TRUE))"
      << "   AND NOT EXISTS (SELECT * FROM data AS d"
      << "                    WHERE d.stationid = o.stationid"
      << "                      AND d.typeid = o.typeid"
      << "                      AND d.paramid = o.paramid"
      << "                      AND d.level = 0 AND d.sensor = '0'" // FIXME which level/sensor to use? (see below)
      << "                      AND d.obstime = t.s"
      << "                      AND NOT (substr(d.controlinfo, 7,1) IN ('1','2','3')" // fmis == 1, 2, 3 (original or corrected missing)
      << "                           AND substr(d.controlinfo,13,1) IN ('0','1')))"   // fd <= 1 (no accumulation)
      << " ORDER BY o.stationid, o.typeid, t.s";
  METLIBS_LOG_DEBUG(LOGVAL(sql.str()));

  std::vector<SensorTime> results;

  QSqlQuery query(hqcApp->kvalobsDB());
  if (query.exec(QString::fromStdString(sql.str()))) {
    while (query.next()) {
      const int stationId = query.value(0).toInt(), typeId = query.value(1).toInt();
      const timeutil::ptime t = timeutil::from_iso_extended_string(query.value(2).toString().toStdString());

      const Sensor s(stationId, kvalobs::PARAMID_RR_24, /*level*/ 0, /*sensor*/0, typeId); // FIXME which level/sensor to use? (see above)
      results.push_back(SensorTime(s, t));
    }
  } else {
    HQC_LOG_ERROR("query '" << sql << "' failed: " << query.lastError().text());
  }

  return results;
}

} // namespace Missing
