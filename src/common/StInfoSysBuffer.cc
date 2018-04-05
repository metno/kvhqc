/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2018 met.no

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

#include "StInfoSysBuffer.hh"

#include "KvMetaDataBuffer.hh"
#include "common/HqcSystemDB.hh"
#include "util/Helpers.hh"
#include "util/stringutil.hh"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>
#include <QVariant>

#include <map>
#include <set>

#define MILOGGER_CATEGORY "kvhqc.StInfoSysBuffer"
#include "util/HqcLogging.hh"

namespace /*anonymous*/ {

const char QSQLNAME_REMOTE[] = "stinfosys";
const int norway_countryid = 578;     // FIXME do not hardcode this
const int norway_remap_municip = 100; // FIXME do not hardcode this
const QString OTHER = "OTHER";

struct municip_info {
  QString municip_name; // norwegian or foreign municipality name
  QString county_name;  // name of norwegian county
  municip_info(QString mn, QString cn)
    : municip_name(mn), county_name(cn) { }
};

} // anonymous namespace

/*!
  Read station info from the stinfosys database
*/
bool StInfoSysBuffer::readFromStInfoSys()
{
  METLIBS_LOG_SCOPE();
  if (not isConnected())
    return false;

  const HqcSystemDB::station2prio_t station2prio = HqcSystemDB::stationPriorities();
  const hqc::int_s station2coast = HqcSystemDB::coastalStations();
  if (station2prio.empty() and station2coast.empty())
    return false;

  typedef std::map<int, municip_info> municip2info_m;
  municip2info_m municip2info;
  {
    QSqlQuery query(QSqlDatabase::database(QSQLNAME_REMOTE));
    if (not query.exec("SELECT m.municipid, m.countryid, m.code, m.name FROM municip m ORDER BY m.municipid")) {
      HQC_LOG_ERROR("cannot read municip table from stinfosys: " << query.lastError().text());
      return false;
    }
    while (query.next()) {
      const int municipid = query.value(0).toInt();
      const int countryid = query.value(1).toInt();
      const int municip_code = query.value(2).toInt();
      QString municip_name = query.value(3).toString();
      QString county_name;
      METLIBS_LOG_DEBUG(LOGVAL(municipid) << LOGVAL(countryid) << LOGVAL(municip_code) << LOGVAL(municip_name));

      if (municipid == 0) {
        county_name = municip_name = OTHER;
      } else {
        QString cn = HqcSystemDB::remappedCounty(countryid, municip_code);
        if (not cn.isNull()) {
          county_name = cn;
          METLIBS_LOG_DEBUG("stationinfo remapped " << LOGVAL(county_name));
        } else if (countryid == norway_countryid and municipid >= norway_remap_municip) {
          // FIXME this is fragile, it depends on the order of the query results
          municip2info_m::const_iterator itC = municip2info.find(municipid / 100);
          if (itC != municip2info.end()) {
            county_name = itC->second.municip_name;
            METLIBS_LOG_DEBUG("norwegian special remapped " << LOGVAL(county_name));
          } else {
            county_name = OTHER;
            HQC_LOG_WARN("no county name for municipid " << municipid << ", using '" << county_name << "'");
          }
        }
      }
      if (county_name.isEmpty()) {
        if (countryid == norway_countryid and municipid >= norway_remap_municip)
          HQC_LOG_WARN("empty county for municipid " << municipid);
        county_name = OTHER;
      }
      if (municip_name.isEmpty())
        municip_name = OTHER;

      municip2info.insert(std::make_pair(municipid, municip_info(municip_name, county_name)));
    }
  }

  const char stinfosys_SQL[] =
      "SELECT DISTINCT s.stationid,"
      "      CASE WHEN (s.municipid IS NOT NULL) THEN s.municipid ELSE 0 END"
      " FROM station s"
      " WHERE s.stationid IN (SELECT o.stationid FROM obs_pgm AS o)";

  QSqlQuery query(QSqlDatabase::database(QSQLNAME_REMOTE));
  if (not query.exec(stinfosys_SQL)) {
    HQC_LOG_ERROR("cannot read station table from stinfosys: " << query.lastError().text());
    return false;
  }

  typedef std::map<int, int> station2municip_m;
  station2municip_m station2municip;
  while (query.next()) {
    const int stationid = query.value(0).toInt();
    const int municipid = query.value(1).toInt();
    station2municip.insert(std::make_pair(stationid, municipid));
  }

  listStat.clear();
  std::set<int> kv_stationids;
  const hqc::kvStation_v& slist = KvMetaDataBuffer::instance()->allStations();
  for (const kvalobs::kvStation& st : slist) {
    const int stationid = st.stationID();

    const station2municip_m::const_iterator itM = station2municip.find(stationid);
    if (itM == station2municip.end()) {
      METLIBS_LOG_DEBUG("no municipid for station " << stationid << "; station ignored");
      continue;
    }
    const int municipid = itM->second;

    const municip2info_m::const_iterator itI = municip2info.find(municipid);
    if (itI == municip2info.end()) {
      METLIBS_LOG_DEBUG("no municip info for station " << stationid << "; station ignored");
      continue;
    }
    const municip_info& mi = itI->second;

    const HqcSystemDB::station2prio_t::const_iterator it = station2prio.find(stationid);
    const int pri = (it != station2prio.end()) ? it->second : 0;

    listStat_t ls;
    ls.name = Helpers::fromUtf8(st.name());
    ls.stationid   = stationid;
    ls.altitude    = st.height();
    ls.wmonr       = st.wmonr();

    ls.fylke = mi.county_name;
    ls.kommune = mi.municip_name;
    ls.municipid   = municipid; // useed in extreme value list to exclude ships
    ls.pri         = pri;
    ls.coast       = (station2coast.find(stationid) != station2coast.end());

    if (kv_stationids.find(stationid) != kv_stationids.end()) {
      METLIBS_LOG_INFO("kvalobs has duplicate stationid " << stationid << ", ignored");
    } else {
      listStat.push_back(ls);
      kv_stationids.insert(stationid);
    }
  }
  METLIBS_LOG_INFO("stationliste hentet fra stinfosys");

  const char manual_types_SQL[] =
      "SELECT message_formatid FROM message_format WHERE read = 'M' ORDER BY message_formatid";

  if (not query.exec(manual_types_SQL)) {
    HQC_LOG_ERROR("query to stinfosys failed: " << query.lastError().text());
    return false;
  }
  mManualTypes.clear();
  while (query.next()) {
    const int typeId = query.value(0).toInt();
    METLIBS_LOG_DEBUG(LOGVAL(typeId));
    mManualTypes.push_back(typeId);
  }

  writeToStationFile();
  return true;
}

StInfoSysBuffer::StInfoSysBuffer(std::shared_ptr<miutil::conf::ConfSection> conf)
{
  Helpers::connect2postgres(QSQLNAME_REMOTE, conf, "stinfosys");
}

StInfoSysBuffer::~StInfoSysBuffer()
{
  QSqlDatabase::removeDatabase(QSQLNAME_REMOTE);
}

bool StInfoSysBuffer::isConnected()
{
  QSqlDatabase db = QSqlDatabase::database(QSQLNAME_REMOTE);
  return db.isOpen();
}

void StInfoSysBuffer::readStationInfo()
{
  if (not readFromStInfoSys())
    StationInfoBuffer::readStationInfo();
}
