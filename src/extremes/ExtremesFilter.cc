/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2014-2018 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: kvalobs-dev@met.no

  This file is part of HQC

  HQC is free software; you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  HQC is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General Public License
  along with HQC; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/


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

  mExcludedIds.clear();
  if (StationInfoBuffer::instance()) {
    const listStat_l& stationDetails = StationInfoBuffer::instance()->getStationDetails();
    for (listStat_l::const_iterator it = stationDetails.begin(); it != stationDetails.end(); ++it) {
      if (Helpers::isNorwegianStationId(it->stationid)
          and (it->municipid <= 100 or it->municipid >= 2100))
      {
        mExcludedIds.insert(it->stationid);
      }
    }
  }
}

QString ExtremesFilter::acceptingSqlExtraTables(const QString&, const TimeSpan& time) const
{
  METLIBS_LOG_SCOPE();

  QString sql = "(SELECT dd.stationid AS s, "
      + QString(mFindMaximum ? "MAX" : "MIN") + "(dd.corrected) AS c "
      + "FROM data AS dd "
      "   WHERE " + QString::fromStdString(Helpers::isNorwegianStationIdSQL("dd.stationid"));

  if (!mExcludedIds.empty())
    sql += " AND NOT (" + set2sql("dd.stationid", mExcludedIds) + ")";
  sql += " AND " + set2sql("dd.paramid", mParamIds);
  sql += " AND " + exists_in_obspgm("dd.");
  sql += " AND (substr(dd.useinfo,3,1) IN ('0','1','2')"
         "        OR (substr(dd.useinfo,3,1) = '3' AND dd.original = dd.corrected))";
  sql += " AND dd.obstime " + timespan2sql(time);

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
  QString sql = d + "stationid = ex.s";
  sql += " AND " + d + "corrected = ex.c";
  sql += " AND " + set2sql(d + "paramid", mParamIds);
  sql += " AND " + exists_in_obspgm(d);
  sql += " ORDER BY " + d + "corrected " + (mFindMaximum ? "DESC" : "ASC") + ", " + d + "stationid, " + d + "obstime";
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
  if (afterSQL)
    return true;

  const int obsStationId = obs->sensorTime().sensor.stationId;
  if (!Helpers::isNorwegianStationId(obsStationId) || mExcludedIds.count(obsStationId))
    return false;

  const int obsParamId = obs->sensorTime().sensor.paramId;
  if (!mParamIds.count(obsParamId))
    return false;

  // FIXME: check obs_pgm

#if 0
  const int ui2 = obs->useinfo().flag(2);
#else
  kvalobs::kvUseInfo obsUI;
  obsUI.setUseFlags(obs->controlinfo());
  const int ui2 = obsUI.flag(2);
#endif
  if (!(ui2 == 0 || ui2 == 1 || ui2 == 2 || (ui2 == 3 && obs->original() == obs->corrected())))
    return false;

  // not needed: check obstime

  // exclude some special values (no measurement, no snow, no rain, ...)
  if (obsParamId == kvalobs::PARAMID_RR_24) {
    if (obs->corrected() == -1)
      return false;
  } else if (obsParamId == kvalobs::PARAMID_SA) {
    if (obs->corrected() == -3 || obs->corrected() == -1 || obs->corrected() == 0)
      return false;
  }

  return true;
}
