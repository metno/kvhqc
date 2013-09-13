
#include "StationInfoBuffer.hh"

#include "KvMetaDataBuffer.hh"
#include "common/gui/HqcApplication.hh"

#include <QtCore/QVariant>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.StationInfoBuffer"
#include "util/HqcLogging.hh"

namespace /*anonymous*/ {

const char STATIONINFO_CACHE[] = "stationinfo_cache";
const char STATIONINFO_CACHE_CREATE[] = "CREATE TABLE stationinfo_cache ("
    "stationid    INTEGER PRIMARY KEY,"
    "municip_id   INTEGER NOT NULL,"
    "county       TEXT    NOT NULL,"
    "municip_name TEXT    NOT NULL,"
    "is_coastal   BOOLEAN NOT NULL,"
    "priority     INTEGER DEFAULT 0"
    ");";

const char STATIONINFO_CACHE_DELETE[] = "DELETE FROM stationinfo_cache;";
const char STATIONINFO_CACHE_INSERT[] = "INSERT INTO stationinfo_cache VALUES"
    " (:sid, :mun_id, :county, :mun_name, :coast, :prio)";

const char STATIONINFO_CACHE_SELECT_ALL[] = "SELECT stationid, municip_id, county, municip_name, is_coastal, priority FROM stationinfo_cache;";

} // anonymous namespace

StationInfoBuffer* StationInfoBuffer::sInstance = 0;

StationInfoBuffer::StationInfoBuffer()
{
    assert(not sInstance);
    sInstance = this;
}

StationInfoBuffer::~StationInfoBuffer()
{
    sInstance = 0;
}

/*!
  Read station info from the stinfosys database
*/
bool StationInfoBuffer::writeToStationFile()
{
  METLIBS_LOG_SCOPE();

  QSqlDatabase db = hqcApp->configDB();
  if (not db.tables().contains(STATIONINFO_CACHE))
    db.exec(STATIONINFO_CACHE_CREATE);

  db.transaction();
  QSqlQuery deleteall(db);
  deleteall.prepare(STATIONINFO_CACHE_DELETE);
  if (not deleteall.exec())
    HQC_LOG_ERROR("error while deleting: " << deleteall.lastError().text());

  QSqlQuery insert(db);
  insert.prepare(STATIONINFO_CACHE_INSERT);
  BOOST_FOREACH(const listStat_t& ls, listStat) {
    const QString qcounty = QString::fromStdString(ls.fylke), qmunicip = QString::fromStdString(ls.kommune);
    insert.bindValue(":sid",      ls.stationid);
    insert.bindValue(":mun_id",   ls.municipid);
    insert.bindValue(":county",   qcounty);
    insert.bindValue(":mun_name", qmunicip);
    insert.bindValue(":prio",     ls.pri);
    insert.bindValue(":coast",    ls.coast);
    if (not insert.exec())
      HQC_LOG_ERROR("error while inserting: " << insert.lastError().text());
    insert.finish();
  }
  db.commit();
  return true;
}

/*!
  Read the station file, this must be done after the station table in the database is read
*/
bool StationInfoBuffer::readFromStationFile()
{
  METLIBS_LOG_SCOPE();
  
  QSqlDatabase db = hqcApp->configDB();
  if (not db.tables().contains(STATIONINFO_CACHE))
    return false;

  QSqlQuery query(STATIONINFO_CACHE_SELECT_ALL, hqcApp->configDB());
  while (query.next()) {
    try {
      const int stationId = query.value(0).toInt();
      const kvalobs::kvStation& st = KvMetaDataBuffer::instance()->findStation(stationId);
      
      listStat_t ls;
      ls.stationid   = stationId;

      ls.name        = st.name();
      ls.altitude    = st.height();
      ls.environment = st.environmentid();
      ls.wmonr       = st.wmonr();
        
      ls.municipid   = query.value(1).toInt();
      ls.fylke       = query.value(2).toString().toStdString();
      ls.kommune     = query.value(3).toString().toStdString();
      ls.coast       = query.value(4).toBool();
      ls.pri         = query.value(5).toInt();

      // FIXME fromtime and totime
        
      listStat.push_back(ls);
    } catch (std::exception& e) {
      HQC_LOG_WARN("exception while reading stationinfo_cache: " << e.what());
    }
  }
  return true;
}

bool StationInfoBuffer::isConnected()
{
  QSqlDatabase db = hqcApp->configDB();
  return (db.tables().contains(STATIONINFO_CACHE));
}

const listStat_l& StationInfoBuffer::getStationDetails()
{
    METLIBS_LOG_SCOPE();

    const timeutil::ptime now = timeutil::now();
    METLIBS_LOG_DEBUG(LOGVAL(now) << LOGVAL(mLastStationListUpdate));
    if (mLastStationListUpdate.is_not_a_date_time()
        or (now - mLastStationListUpdate).total_seconds() > 3600)
    {
        mLastStationListUpdate = now;
        readStationInfo();
    }
    return listStat;
}

void StationInfoBuffer::readStationInfo()
{
    readFromStationFile();
}
