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

#include "HqcSystemDB.hh"

#include "common/HqcApplication.hh"

#include <QCoreApplication>
#include <QLocale>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

#define MILOGGER_CATEGORY "kvhqc.HqcSystemDB"
#include "util/HqcLogging.hh"

QString HqcSystemDB::explainCheck(QString& check)
{
  if (not hqcApp)
    return "?";

  if (check.startsWith("QC2N_")) {
    QString n = check.mid(5);
    n.replace("_", ", ");
    n.prepend(qApp->translate("ChecksTableModel", "neighbors: "));
    check = "QC2-redist-N";
    return n;
  }

  QSqlQuery query(hqcApp->systemDB());
  query.prepare("SELECT description FROM check_explain WHERE qcx = ? AND language = 'nb'");
  query.bindValue(0, check);
  query.exec();
  if (query.next())
    return query.value(0).toString();
  else
    return "?";
}

bool HqcSystemDB::paramLimits(int paramid, float& low, float& high)
{
  if (not hqcApp)
    return false;

  QSqlQuery query(hqcApp->systemDB());
  query.exec("SELECT low, high FROM slimits WHERE paramid = ?");
  query.bindValue(0, paramid);
  query.exec();
  if (query.next()) {
    low = query.value(0).toFloat();
    high = query.value(1).toFloat();
    return true;
  }
  return false;
}

bool HqcSystemDB::shownDecimals(int paramid, int& decimals)
{
  if (not hqcApp)
    return false;

  QSqlQuery queryDecimals(hqcApp->systemDB());
  queryDecimals.prepare("SELECT decimals FROM param_decimals WHERE paramid = ?");
  queryDecimals.bindValue(0, paramid);
  queryDecimals.exec();
  if (queryDecimals.next()) {
    decimals = queryDecimals.value(0).toInt();
    return true;
  }
  return false;
}

namespace /* anonymous */ {

QString languageOrdering(QString languageColumn)
{
  METLIBS_LOG_SCOPE();
  QStringList uil = QLocale::system().uiLanguages();
  if (uil.isEmpty())
    uil << "C";

  QString lo = "(CASE " + languageColumn;
  for (int i=0; i<uil.size(); ++i)
    lo += QString(" WHEN '%1' THEN %2").arg(uil.at(i)).arg(i);
  lo += QString(" ELSE %1 END) ASC").arg(uil.size());
  METLIBS_LOG_DEBUG(LOGVAL(lo));
  return lo;
}

} // namespace anonymous

HqcSystemDB::ParamCode_ql HqcSystemDB::paramCodes(int paramid)
{
  ParamCode_ql pcl;
  
  if (hqcApp) {
    const QString langOrder = languageOrdering("language");
    
    QSqlQuery queryCodes(hqcApp->systemDB());
    queryCodes.prepare("SELECT id, code_value FROM param_codes"
        " WHERE paramid = ? ORDER BY code_value ASC");
    
    QSqlQuery queryCodeLong(hqcApp->systemDB());
    queryCodeLong.prepare("SELECT long_text, language FROM param_code_long"
        " WHERE code_id = ? ORDER BY " + langOrder + " LIMIT 1");
    
    QSqlQuery queryCodeShort(hqcApp->systemDB());
    queryCodeShort.prepare("SELECT short_text FROM param_code_short"
        " WHERE code_id = ? AND language = ? ORDER BY sortkey ASC");
    
    queryCodes.bindValue(0, paramid);
    if (queryCodes.exec()) {
      while (queryCodes.next()) {
        ParamCode pc;

        const int code_id = queryCodes.value(0).toInt();
        pc.value = queryCodes.value(1).toInt();
        
        queryCodeLong.bindValue(0, code_id);
        if (queryCodeLong.exec()) {
          if (queryCodeLong.next()) {
            pc.longText = queryCodeLong.value(0).toString();
            
            const QString language = queryCodeLong.value(1).toString();
            
            queryCodeShort.bindValue(0, code_id);
            queryCodeShort.bindValue(1, language);
            if (not queryCodeShort.exec())
              HQC_LOG_WARN("error getting short text for parameter " << paramid << ", code " << pc.value
                  << " and language '" << language << "' from system DB: " << queryCodeShort.lastError().text());
            while (queryCodeShort.next())
              pc.shortTexts << queryCodeShort.value(0).toString();
          }
          pcl << pc;
        } else {
          HQC_LOG_WARN("error getting long text text for parameter " << paramid << ", code " << pc.value
              << " from system DB: " << queryCodeLong.lastError().text());
        }
      }
    } else {
      HQC_LOG_ERROR("error getting code values for parameter " << paramid
          << " from system DB: " << queryCodes.lastError().text());
    }
  }
  return pcl;
}

HqcSystemDB::station2prio_t HqcSystemDB::stationPriorities()
{
  station2prio_t priorities;
  if (not hqcApp)
    return priorities;

  QSqlDatabase db = hqcApp->systemDB();
  if (not db.tables().contains("stationinfo_priorities"))
    return priorities;

  QSqlQuery query(db);
  if (not query.exec("SELECT stationid, priority FROM stationinfo_priorities")) {
    HQC_LOG_ERROR("cannot read priorities: " << query.lastError().text());
    return priorities;
  }

  while (query.next()) {
    const int stationId = query.value(0).toInt(), prio = query.value(1).toInt();
    priorities.insert(std::make_pair(stationId, prio));
  }
  return priorities;
}

hqc::int_s HqcSystemDB::coastalStations()
{
  hqc::int_s coastal;
  if (not hqcApp)
    return coastal;

  QSqlQuery query(hqcApp->systemDB());
  if (not query.exec("SELECT stationid FROM stationinfo_coastal")) {
    HQC_LOG_ERROR("cannot read coastal station list: " << query.lastError().text());
    return coastal;
  }

  while (query.next())
    coastal.insert(query.value(0).toInt());
  return coastal;
}

QString HqcSystemDB::remappedCounty(int countryid, int municip_code)
{
  QSqlQuery queryNCFC(hqcApp->systemDB());
  queryNCFC.prepare("SELECT norwegian_county FROM stationinfo_county_map WHERE countryid = ?"
      " AND municip_code_divided = (? / municip_divide)");
  queryNCFC.bindValue(0, countryid);
  queryNCFC.bindValue(1, municip_code);

  if (not queryNCFC.exec()) {
    HQC_LOG_ERROR("cannot read stationinfo_county_map: " << queryNCFC.lastError().text());
    return QString();
  }
  if (queryNCFC.next()) {
    QString county_name = queryNCFC.value(0).toString();
    METLIBS_LOG_DEBUG("stationinfo remapped " << LOGVAL(county_name));
    return county_name;
  }
  return QString();
}

QString HqcSystemDB::explainFlagValue(int fn, int fv)
{
  if (not hqcApp)
    return QString();

  QSqlQuery query(hqcApp->systemDB());
  query.prepare("SELECT description FROM flag_explain WHERE flag = :fn AND flagvalue = :fv AND language = 'nb'");
  query.bindValue(":fn", fn);
  query.bindValue(":fv", fv);
  if (query.exec()) {
    if (query.next())
      return query.value(0).toString();
  } else {
    HQC_LOG_WARN("error getting flag explanation for flag=" << fn << " value=" << fv << ": " << query.lastError().text());
  }
  return QString();
}

hqc::int_v HqcSystemDB::relatedParameters(int paramid, const QString& viewType)
{
  hqc::int_v related;
  if (hqcApp) {
    QSqlQuery query(hqcApp->systemDB());
    query.prepare("SELECT pr1.paramid FROM param_related AS pr1"
        " WHERE pr1.groupid = (SELECT pr2.groupid FROM param_related AS pr2 WHERE pr2.paramid = :pid)"
        "   AND (pr1.view_types_excluded IS NULL OR pr1.view_types_excluded NOT LIKE :vt)"
        " ORDER BY pr1.sortkey");

    query.bindValue(":pid", paramid);
    query.bindValue(":vt",  "%" + viewType + "%");
    if (query.exec()) {
      query.exec();
      while (query.next())
        related.push_back(query.value(0).toInt());
    } else {
      HQC_LOG_WARN("error getting related parameters for paramid=" << paramid << " viewtype=" << viewType << ": " << query.lastError().text());
    }
  }
  return related;
}

hqc::int_s HqcSystemDB::ignoredParameters(const QString& viewType)
{
  hqc::int_s ignored;
  if (hqcApp) {
    QSqlQuery query(hqcApp->systemDB());
    query.prepare("SELECT paramid FROM param_ignored"
                  " WHERE (view_type IS NULL) OR (view_type = :vt)");

    query.bindValue(":vt", viewType);
    if (query.exec()) {
      while (query.next())
        ignored.insert(query.value(0).toInt());
    } else {
      HQC_LOG_WARN("error getting ignored parameters for viewtype=" << viewType << ": " << query.lastError().text());
    }
  }
  return ignored;
}

void HqcSystemDB::aggregatedParameters(int paramFrom, hqc::int_s& paramTo)
{
  METLIBS_LOG_SCOPE();
  if (hqcApp) {
    QSqlQuery query(hqcApp->systemDB());
    query.prepare("SELECT paramid_to FROM param_aggregated WHERE paramid_from = ?");
    query.bindValue(0, paramFrom);
    if (query.exec()) {
      while (query.next())
        paramTo.insert(query.value(0).toInt());
    } else {
      HQC_LOG_WARN("error getting parameters aggregating from " << paramFrom
          << ": " << query.lastError().text());
    }
  }
}

void HqcSystemDB::aggregatedParameters(hqc::int_s& paramFrom, int paramTo)
{
  METLIBS_LOG_SCOPE();
  if (hqcApp) {
    QSqlQuery query(hqcApp->systemDB());
    query.prepare("SELECT paramid_from FROM param_aggregated WHERE paramid_to = ?");
    query.bindValue(0, paramTo);
    if (query.exec()) {
      while (query.next())
        paramFrom.insert(query.value(0).toInt());
    } else {
      HQC_LOG_WARN("error getting parameters aggregating to " << paramTo
          << ": " << query.lastError().text());
    }
  }
}

HqcSystemDB::Region_ql HqcSystemDB::regions()
{
  Region_ql rl;

  QSqlQuery queryRegions(hqcApp->systemDB());
  queryRegions.prepare("SELECT sr.id, srl.label FROM station_regions AS sr, station_region_labels AS srl"
      " WHERE sr.id = srl.region_id AND srl.language = 'nb' ORDER BY sr.sortkey");
    
  QSqlQuery queryCounties(hqcApp->systemDB());
  queryCounties.prepare("SELECT sc.db_name, scl.label FROM station_county_labels AS scl, station_counties AS sc"
      " WHERE sc.id = scl.county_id AND scl.language = 'nb' AND sc.region_id = ? ORDER BY sc.sortkey");
    
  queryRegions.exec();
  while (queryRegions.next()) {
    Region r;
    const int regionId = queryRegions.value(0).toInt();
    r.regionLabel = queryRegions.value(1).toString();

    queryCounties.bindValue(0, regionId);
    queryCounties.exec();
    while (queryCounties.next()) {
      r.countyDbNames << queryCounties.value(0).toString();
      r.countyLabels  << queryCounties.value(1).toString();
    }
    rl << r;
  }
  return rl;
}

HqcSystemDB::ParamGroup_ql HqcSystemDB::paramGroups()
{
  ParamGroup_ql pgl;

  QSqlQuery queryGroups(hqcApp->systemDB());
  queryGroups.prepare("SELECT pg.id, pgl.label FROM param_groups AS pg, param_group_labels AS pgl"
      " WHERE pg.id = pgl.group_id AND pgl.language = 'nb' ORDER BY pg.sortkey");
    
  QSqlQuery queryParams(hqcApp->systemDB());
  queryParams.prepare("SELECT paramid, auxiliary FROM param_order WHERE group_id = ? ORDER BY sortkey");
    
  queryGroups.exec();
  while (queryGroups.next()) {
    ParamGroup pg;
    const int groupId = queryGroups.value(0).toInt();
    pg.label = queryGroups.value(1).toString();
    METLIBS_LOG_DEBUG(LOGVAL(pg.label));

    queryParams.bindValue(0, groupId);
    queryParams.exec();
    while (queryParams.next()) {
      ParamGroupEntry p;
      p.paramId = queryParams.value(0).toInt();
      p.auxiliary = queryParams.value(1).toBool();
      pg.paramIds.push_back(p);
    }

    pgl << pg;
  }
  return pgl;
}
