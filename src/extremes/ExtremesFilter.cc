
#include "ExtremesFilter.hh"

#include "common/KvHelpers.hh"
#include "common/sqlutil.hh"
#include "common/StationInfoBuffer.hh"

#define MILOGGER_CATEGORY "kvhqc.ExtremesFilter"
#include "util/HqcLogging.hh"

// ************************************************************************

namespace /*anonymous*/ {
QString exists_in_obspgm(const QString& data_alias, const QString& obs_pgm_alias = "o")
{
  const QString &d = data_alias, &o = obs_pgm_alias;
  return "EXISTS (SELECT * FROM obs_pgm AS " + o + " WHERE " + o + ".stationid = " + d + ".stationid"
      " AND " + o + ".paramid = " + d + ".paramid"
      " AND " + o + ".typeid  = " + d + ".typeid"
      " AND " + o + ".fromtime <= " + d + ".obstime"
      " AND (" + o + ".totime IS NULL OR " + o + ".totime >= " + d + ".obstime))";
}
} // namespace anonymous

void ExtremesFilter::prepareParams(QString& paramIds, QString& function, QString& ordering) const
{
  bool findMaximum = true;
  if (mParamId == kvalobs::PARAMID_TAN) {
    // TA, TAN, TAN_12
    paramIds = "211,213,214";
    findMaximum = false;
  } else if (mParamId == kvalobs::PARAMID_TAX) {
    // TA, TAX, TAX_12
    paramIds = "211,215,216";
  } else if (mParamId == kvalobs::PARAMID_FG) {
    // FG, FG_010, FG_1, FG_6, FG_12, FG_X
    paramIds = "83,84,90,91,92,94";
  } else if (mParamId == kvalobs::PARAMID_FX) {
    // FX, FX_1, FX_6, FX_12, FX_X, FX_3
    paramIds = "86,87,88,89,93,95";
  } else {
    paramIds = QString::number(mParamId);
  }
  function = findMaximum ? "MAX"  : "MIN";
  ordering = findMaximum ? "DESC" : "ASC";
}

QString ExtremesFilter::acceptingSqlExtraTables(const QString& d, const TimeSpan& time) const
{
  METLIBS_LOG_SCOPE();

  const listStat_l& stationDetails = StationInfoBuffer::instance()->getStationDetails();
  QString excludedIds, sep = "";
  for (listStat_l::const_iterator it = stationDetails.begin(); it != stationDetails.end(); ++it) {
    if (Helpers::isNorwegianStationId(it->stationid)
        and (it->municipid <= 100 or it->municipid >= 2100))
    {
      excludedIds += sep + QString::number(it->stationid);
      sep = ",";
    }
  }
  METLIBS_LOG_DEBUG(LOGVAL(excludedIds));

  QString paramIds, function, ordering;
  prepareParams(paramIds, function, ordering);

  QString sql = "SELECT dd.stationid AS s, " + function + "(dd.corrected) AS c FROM data AS dd "
      "   WHERE " + QString::fromStdString(Helpers::isNorwegianStationIdSQL("dd.stationid"));
  if (not excludedIds.isEmpty())
    sql += " AND dd.stationid NOT IN (" + excludedIds + ")";
  sql += "   AND dd.paramid IN (" + paramIds + ")"
      "   AND " + exists_in_obspgm("dd") +
      "   AND (substr(dd.useinfo,3,1) IN ('0','1','2')"
      "        OR (substr(dd.useinfo,3,1) = '3' AND dd.original = dd.corrected))"
      "   AND dd.obstime " + timespan2sql(time);

  // exclude some special values (no measurement, no snow, no rain, ...)
  if (mParamId == kvalobs::PARAMID_RR_24)
    sql += "   AND dd.corrected != -1";
  else if (mParamId == kvalobs::PARAMID_SA)
    sql += "   AND dd.corrected != -3 AND dd.corrected != -1 AND dd.corrected != 0";

  sql += QString(" GROUP BY s ORDER BY c %1 LIMIT %2) AS ex").arg(ordering).arg(mExtremesCount);
  METLIBS_LOG_DEBUG(LOGVAL(sql));
  return sql;
}

QString ExtremesFilter::acceptingSql(const QString& d, const TimeSpan&) const
{
  METLIBS_LOG_SCOPE();

  QString paramIds, function, ordering;
  prepareParams(paramIds, function, ordering);

  QString sql = " WHERE stationid = ex.s"
      " AND corrected = ex.c"
      " AND paramid IN (" + paramIds + ")"
      " AND " + exists_in_obspgm(d) +
      " ORDER BY corrected " + ordering + ", stationid, obstime";
  METLIBS_LOG_DEBUG(LOGVAL(sql));
  return sql;
}

bool ExtremesFilter::subsetOf(ObsFilter_p other) const
{
  ExtremesFilter_p oe = boost::dynamic_pointer_cast<ExtremesFilter>(other);
  if (not oe)
    return false;
  if (oe->mParamId != this->mParamId
      or oe->mExtremesCount != this->mExtremesCount)
    return false;
  return true;
}
