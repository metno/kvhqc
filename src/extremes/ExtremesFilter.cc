
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
  return "EXISTS (SELECT * FROM obs_pgm AS " + o + " WHERE " + o + ".stationid = " + d + "stationid"
      " AND " + o + ".paramid = " + d + "paramid"
      " AND " + o + ".typeid  = " + d + "typeid"
      " AND " + o + ".fromtime <= " + d + "obstime"
      " AND (" + o + ".totime IS NULL OR " + o + ".totime >= " + d + "obstime))";
}
} // namespace anonymous

ExtremesFilter::ExtremesFilter(int paramid, int nExtremes)
  : mParamId(paramid)
  , mExtremesCount(nExtremes)
  , mFindMaximum(true)

{
  mParamIds.insert(mParamId);
  if (mParamId == kvalobs::PARAMID_TAN) {
    // TA, TAN, TAN_12
    mParamIds.insert(kvalobs::PARAMID_TA);
    mParamIds.insert(kvalobs::PARAMID_TAN_12);
    mFindMaximum = false;
  } else if (mParamId == kvalobs::PARAMID_TAX) {
    // TA, TAX, TAX_12
    mParamIds.insert(kvalobs::PARAMID_TA);
    mParamIds.insert(kvalobs::PARAMID_TAX_12);
  } else if (mParamId == kvalobs::PARAMID_FG) {
    // FG, FG_010, FG_1, FG_6, FG_12, FG_X
    const int np=5, p[np] = { 84,90,91,92,94 };
    mParamIds.insert(p, p+np);
  } else if (mParamId == kvalobs::PARAMID_FX) {
    // FX, FX_1, FX_6, FX_12, FX_X, FX_3
    const int np=5, p[np] = { 87,88,89,93,95 };
    mParamIds.insert(p, p+np);
  }
}

QString ExtremesFilter::findExcludedIds() const
{
  QString excludedIds;
  if (StationInfoBuffer::instance()) {
    bool first = true;
    const QString sep = ",";
    const listStat_l& stationDetails = StationInfoBuffer::instance()->getStationDetails();
    for (listStat_l::const_iterator it = stationDetails.begin(); it != stationDetails.end(); ++it) {
      if (Helpers::isNorwegianStationId(it->stationid)
          and (it->municipid <= 100 or it->municipid >= 2100))
      {
        if (first)
          first = false;
        else
          excludedIds += sep;
        excludedIds += QString::number(it->stationid);
      }
    }
  }
  return excludedIds;
}


QString ExtremesFilter::acceptingSqlExtraTables(const QString& d, const TimeSpan& time) const
{
  METLIBS_LOG_SCOPE();

  QString sql = "(SELECT dd.stationid AS s, "
      + QString(mFindMaximum ? "MAX" : "MIN") + "(dd.corrected) AS c "
      + "FROM data AS dd "
      "   WHERE " + QString::fromStdString(Helpers::isNorwegianStationIdSQL("dd.stationid"));

  const QString excludedIds = findExcludedIds();
  if (not excludedIds.isEmpty())
    sql += " AND dd.stationid NOT IN (" + excludedIds + ")";

  sql += "   AND " + set2sql("dd.paramid", mParamIds) +
      "   AND " + exists_in_obspgm("dd.") +
      "   AND (substr(dd.useinfo,3,1) IN ('0','1','2')"
      "        OR (substr(dd.useinfo,3,1) = '3' AND dd.original = dd.corrected))"
      "   AND dd.obstime " + timespan2sql(time);

  // exclude some special values (no measurement, no snow, no rain, ...)
  if (mParamId == kvalobs::PARAMID_RR_24)
    sql += "   AND dd.corrected != -1";
  else if (mParamId == kvalobs::PARAMID_SA)
    sql += "   AND dd.corrected != -3 AND dd.corrected != -1 AND dd.corrected != 0";

  sql += QString(" GROUP BY s ORDER BY c %1 LIMIT %2) AS ex")
      .arg(mFindMaximum ? "DESC" : "ASC")
      .arg(mExtremesCount);
  METLIBS_LOG_DEBUG(LOGVAL(sql));
  return sql;
}

QString ExtremesFilter::acceptingSql(const QString& d, const TimeSpan&) const
{
  QString sql = d + "stationid = ex.s"
      " AND " + d + "corrected = ex.c"
      " AND " + set2sql(d + "paramid", mParamIds) +
      " AND " + exists_in_obspgm(d) +
      " ORDER BY " + d + "corrected " + (mFindMaximum ? "DESC" : "ASC") + ", " + d + "stationid, " + d + "obstime";
  return sql;
}

bool ExtremesFilter::subsetOf(ObsFilter_p other) const
{
  ExtremesFilter_p oe = std::dynamic_pointer_cast<ExtremesFilter>(other);
  if (not oe)
    return false;
  if (oe->mParamId != this->mParamId
      or oe->mExtremesCount != this->mExtremesCount)
    return false;
  return true;
}

bool ExtremesFilter::accept(ObsData_p obs, bool afterSQL) const
{
  if (afterSQL or mParamIds.count(obs->sensorTime().sensor.paramId))
    return true;
  return false;
}
