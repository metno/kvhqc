
#include "StationInfoBuffer.hh"

#include "KvMetaDataBuffer.hh"
#include "common/HqcApplication.hh"
#include "util/stringutil.hh"

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

const char MANUAL_TYPES_CACHE[] = "manual_types_cache";
const char MANUAL_TYPES_CACHE_CREATE[] = "CREATE TABLE manual_types_cache ("
    "  typeid    INTEGER"
    ");";

const char MANUAL_TYPES_CACHE_DELETE[] = "DELETE FROM manual_types_cache;";
const char MANUAL_TYPES_CACHE_INSERT[] = "INSERT INTO manual_types_cache VALUES"
    " (:typeid)";

const char MANUAL_TYPES_CACHE_SELECT_ALL[] = "SELECT typeid FROM manual_types_cache ORDER BY typeid;";

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
    HQC_LOG_ERROR("error while deleting stationinfo: " << deleteall.lastError().text());

  QSqlQuery insert(db);
  insert.prepare(STATIONINFO_CACHE_INSERT);
  BOOST_FOREACH(const listStat_t& ls, listStat) {
    insert.bindValue(":sid",      ls.stationid);
    insert.bindValue(":mun_id",   ls.municipid);
    insert.bindValue(":county",   ls.fylke);
    insert.bindValue(":mun_name", ls.kommune);
    insert.bindValue(":prio",     ls.pri);
    insert.bindValue(":coast",    ls.coast);
    if (not insert.exec())
      HQC_LOG_ERROR("error while inserting stationinfo for " << ls.stationid
          << ": " << insert.lastError().text());
  }
  db.commit();

  if (not db.tables().contains(MANUAL_TYPES_CACHE))
    db.exec(MANUAL_TYPES_CACHE_CREATE);

  db.transaction();
  QSqlQuery deleteall_mc(db);
  deleteall_mc.prepare(MANUAL_TYPES_CACHE_DELETE);
  if (not deleteall_mc.exec())
    HQC_LOG_ERROR("error while deleting manual type: " << deleteall_mc.lastError().text());

  QSqlQuery insert_mc(db);
  insert_mc.prepare(MANUAL_TYPES_CACHE_INSERT);
  BOOST_FOREACH(int t, mManualTypes) {
    insert_mc.bindValue(":typeid", t);
    if (not insert_mc.exec())
      HQC_LOG_ERROR("error while inserting manual type " << t << ": " << insert_mc.lastError().text());
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
  if (db.tables().contains(STATIONINFO_CACHE)) {
    QSqlQuery query(STATIONINFO_CACHE_SELECT_ALL, db);
    while (query.next()) {
      try {
        const int stationId = query.value(0).toInt();
        const kvalobs::kvStation& st = KvMetaDataBuffer::instance()->findStation(stationId);
        
        listStat_t ls;
        ls.stationid   = stationId;

        ls.name        = Helpers::fromUtf8(st.name());
        ls.altitude    = st.height();
        ls.wmonr       = st.wmonr();
          
        ls.municipid   = query.value(1).toInt();
        ls.fylke       = query.value(2).toString();
        ls.kommune     = query.value(3).toString();
        ls.coast       = query.value(4).toBool();
        ls.pri         = query.value(5).toInt();
  
        listStat.push_back(ls);
      } catch (std::exception& e) {
        HQC_LOG_WARN("exception while reading stationinfo_cache: " << e.what());
      }
    }
  }
  
  if (db.tables().contains(MANUAL_TYPES_CACHE)) {
    mManualTypes.clear();
    QSqlQuery query(MANUAL_TYPES_CACHE_SELECT_ALL, db);
    while (query.next()) {
      mManualTypes.push_back(query.value(0).toInt());
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
  refreshIfOld();
  return listStat;
}

const StationInfoBuffer::manual_types_t& StationInfoBuffer::getManualTypes()
{
  refreshIfOld();
  return mManualTypes;
}

void StationInfoBuffer::refreshIfOld()
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
}

void StationInfoBuffer::readStationInfo()
{
  readFromStationFile();
}
