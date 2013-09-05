
#include "StInfoSysBuffer.hh"

#include "HqcApplication.hh"
#include "hqc_utilities.hh"
#include "KvMetaDataBuffer.hh"

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

#include <boost/foreach.hpp>

#include <map>
#include <set>

#define MILOGGER_CATEGORY "kvhqc.StInfoSysBuffer"
#include "HqcLogging.hh"

namespace /*anonymous*/ {

const char QSQLNAME_REMOTE[] = "stinfosys";

struct countyInfo {
  int stnr;
  int municipid;
  QString name;
  QString county;
  QString municip;
  int pri;
  bool coast;
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

  QSqlDatabase db = hqcApp->systemDB();
  if (not db.tables().contains("stationinfo_priorities"))
    return false;

  typedef std::map<int, int> station2prio_t;
  station2prio_t station2prio;
  {
    QSqlQuery query(db);
    if (not query.exec("SELECT stationid, priority FROM stationinfo_priorities")) {
      METLIBS_LOG_ERROR("cannot read priorities: " << query.lastError().text());
      return false;
    }
    while (query.next()) {
      const int stationId = query.value(0).toInt(), prio = query.value(1).toInt();
      station2prio.insert(std::make_pair(stationId, prio));
    }
  }

  typedef std::set<int> station2coast_t;
  station2coast_t station2coast;
  {
    QSqlQuery query(db);
    if (not query.exec("SELECT stationid FROM stationinfo_coastal")) {
      METLIBS_LOG_ERROR("cannot read coastal station list: " << query.lastError().text());
      return false;
    }
    while (query.next()) {
      const int stationId = query.value(0).toInt();
      station2coast.insert(stationId);
    }
  }

  const char stinfosys_SQL[] =
      "SELECT DISTINCT s.stationid, s.municipid,"
      "      CASE WHEN (s.municipid IS NOT NULL) THEN (SELECT mm.name FROM municip mm"
      "       WHERE s.municipid = mm.municipid) ELSE 'OTHER' END,"
      "      CASE WHEN (s.municipid IS NOT NULL) THEN (SELECT mm.name FROM municip mm"
      "       WHERE ((s.municipid NOT IN (2300,2800) AND mm.municipid = s.municipid / 100)"
      "           OR (s.municipid     IN (2300,2800) AND mm.municipid = s.municipid))) ELSE 'OTHER' END"
      " FROM station s"
      " WHERE s.stationid IN (SELECT o.stationid FROM obs_pgm AS o)"
      " ORDER by s.stationid;";

  QSqlQuery query(QSqlDatabase::database(QSQLNAME_REMOTE));
  if (not query.exec(stinfosys_SQL)) {
    METLIBS_LOG_ERROR("query to stinfosys failed: " << query.lastError().text());
    return false;
  }
  typedef std::map<int,countyInfo> cList_t;
  cList_t cList;
  METLIBS_LOG_INFO("got " << query.size() << " station city/county names from stinfosys");
  while (query.next()) {
    countyInfo cInfo;
    cInfo.stnr      = query.value(0).toInt();
    cInfo.municipid = query.value(1).toInt();
    cInfo.municip   = query.value(2).toString();
    cInfo.county    = query.value(3).toString();
    
    if (cInfo.municipid == 0) {
      cInfo.county   = "OTHER";
      cInfo.municip  = "OTHER";
    } else if (cInfo.municipid >= 2100 and cInfo.municipid < 2300) {
      cInfo.county = "ISHAVET";
    } else if (cInfo.municipid >= 2300 and cInfo.municipid < 2800) {
      cInfo.county  = "MARITIME";
    }
    if (cInfo.county.size() < 3)
      METLIBS_LOG_WARN("empty county for station " << cInfo.stnr);
  
    station2prio_t::const_iterator it = station2prio.find(cInfo.stnr);
    if (it != station2prio.end())
      cInfo.pri = it->second;
    else
      cInfo.pri = 0;
    cInfo.coast = (station2coast.find(cInfo.stnr) != station2coast.end());
    
    METLIBS_LOG_DEBUG(LOGVAL(cInfo.stnr) << LOGVAL(cInfo.municip) << LOGVAL(cInfo.county));
    cList[cInfo.stnr] = cInfo;
  }

  {
    QSqlQuery query(db);
    if (not query.exec("SELECT stationid, county, municipality, is_coastal, priority FROM stationinfo_foreign")) {
      METLIBS_LOG_ERROR("cannot read foreign stations: " << query.lastError().text());
      return false;
    }
    while (query.next()) {
      const int stnr = query.value(0).toInt();

      cList_t::iterator cit = cList.find(stnr);
      if (cit == cList.end())
        continue;

      countyInfo& ci = cit->second;
      ci.municipid = -1;
      ci.county  = query.value(1).toString();
      ci.municip = query.value(2).toString();
      ci.pri     = query.value(3).toBool();
      ci.coast   = query.value(4).toInt();
    }
  }

  listStat.clear();
  const std::list<kvalobs::kvStation> slist = KvMetaDataBuffer::instance()->allStations();
  BOOST_FOREACH(const kvalobs::kvStation& st, slist) {
    cList_t::const_iterator cit = cList.find(st.stationID());
    if (cit == cList.end()) {
      METLIBS_LOG_DEBUG("no county info for station " << st.stationID() << "; might be not in stinfosys.obs_pgm");
      continue;
    }
    const countyInfo& ci = cit->second;
    
    listStat_t ls;
    ls.name        = st.name();
    ls.stationid   = st.stationID();
    ls.altitude    = st.height();
    ls.environment = st.environmentid();
    ls.wmonr       = st.wmonr();
    
    ls.fylke       = ci.county.toStdString();
    ls.kommune     = ci.municip.toStdString();
    ls.municipid   = ci.municipid;
    ls.pri         = ci.pri;
    ls.coast       = ci.coast;
    
    listStat.push_back(ls);
  }
  METLIBS_LOG_INFO("stationliste hentet fra stinfosys");
  writeToStationFile();
  return true;
}

StInfoSysBuffer::StInfoSysBuffer(miutil::conf::ConfSection *conf)
{
  Helpers::connect2postgres(QSQLNAME_REMOTE, conf, "stinfosys");
}

StInfoSysBuffer::~StInfoSysBuffer()
{
  QSqlDatabase::removeDatabase(QSQLNAME_REMOTE);
}

bool StInfoSysBuffer::isConnected()
{
  QSqlDatabase db = QSqlDatabase::database("stinfosys");
  return db.isOpen();
}

void StInfoSysBuffer::readStationInfo()
{
  if (not readFromStInfoSys())
    StationInfoBuffer::readStationInfo();
}
