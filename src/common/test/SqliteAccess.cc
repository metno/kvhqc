
#include "SqliteAccess.hh"
#include "SqliteAccessPrivate.hh"

#include "KvalobsData.hh"
#include "KvalobsUpdate.hh"
#include "SimpleBuffer.hh"

#include "sqlutil.hh"

#include "common/KvHelpers.hh"

#include <boost/algorithm/string.hpp>
#include <boost/make_shared.hpp>

#include <sqlite3.h>

#include <fstream>

#define MILOGGER_CATEGORY "kvhqc.SqliteAccess"
#include "common/ObsLogging.hh"

namespace /*anonymous*/ {
std::string my_sqlite3_string(sqlite3_stmt *stmt, int col)
{
  const char* c = (const char*)sqlite3_column_text(stmt, col);
  if (not c)
    return "";
  else
    return c;
}

Time my_sqlite3_time(sqlite3_stmt *stmt, int col)
{
  return timeutil::from_iso_extended_string((const char*)(sqlite3_column_text(stmt, col)));
}

std::string sql4insert(const kvalobs::kvData& d)
{
  std::ostringstream sql;
  sql << "INSERT INTO data VALUES("
      << d.stationID() << ", "
      << time2sql(d.obstime()) << ", "
      << d.original() << ", "
      << d.paramID() << ", "
      << time2sql(d.tbtime()) << ", "
      << d.typeID() << ", "
      << "'" << d.sensor() << "',"
      << d.level() << ", "
      << d.corrected() << ", "
      << "'" << d.controlinfo() << "', "
      << "'" << d.useinfo() << "', "
      << "'" << d.cfailed() << "')";
  return sql.str();
}

std::string sql4update(const kvalobs::kvData& d)
{
  std::ostringstream sql;
  sql << "UPDATE data SET "
      << " corrected = " << d.corrected() << ", "
      << " controlinfo = '" << d.controlinfo().flagstring() << "', "
      << " useinfo = '" << d.useinfo().flagstring() << "', "
      << " cfailed = '" << d.cfailed() << "' "
      << "WHERE stationid = " << d.stationID()
      << "  AND obstime = " << time2sql(d.obstime())
      << "  AND tbtime = " << time2sql(d.tbtime())
      // << "  AND original = " << d.original()
      << "  AND paramid = " << d.paramID()
      << "  AND typeid = " << d.typeID()
      << "  AND sensor = '" << d.sensor() << "'"
      << "  AND level = " << d.level();
  return sql.str();
}

typedef std::vector<kvalobs::kvData> kvData_v;

} // namespace anonymous

// ========================================================================

SqliteHandler::SqliteHandler()
{
  METLIBS_LOG_SCOPE();
  if (sqlite3_open("", &db))
    throw std::runtime_error("could not create sqlite memory db");
}

// ------------------------------------------------------------------------

SqliteHandler::~SqliteHandler()
{
  METLIBS_LOG_SCOPE();
  sqlite3_close(db);
}

// ------------------------------------------------------------------------

void SqliteHandler::queryData(ObsRequest_p request)
{
  METLIBS_LOG_SCOPE();

  const Sensor_s& sensors = request->sensors();
  const TimeSpan& time = request->timeSpan();
  ObsFilter_p filter   = request->filter();

  ObsData_pv data;
  
  std::ostringstream sql;
  sql << "SELECT d.stationid, d.paramid, d.typeid, d.level, d.sensor,"
      " d.obstime, d.original, d.tbtime, d.corrected, d.controlinfo, d.useinfo, d.cfailed"
      " FROM data d WHERE ";
  sensors2sql(sql, sensors, "d.");
  sql << " AND d.obstime BETWEEN " << time2sql(time.t0()) << " AND " << time2sql(time.t1());
  if (filter and filter->hasSQL())
    sql << " AND (" << filter->acceptingSQL("d.") << ")";
  sql << " ORDER BY d.stationid, d.paramid, d.typeid, d.level, d.sensor, d.obstime";
  //METLIBS_LOG_DEBUG(LOGVAL(sql.str()));

  sqlite3_stmt *stmt = prepare_statement(sql.str());

  int step;
  while( (step = sqlite3_step(stmt)) == SQLITE_ROW ) {
    int col = 0;

    const int stationid = sqlite3_column_int(stmt, col++);
    const int paramid = sqlite3_column_int(stmt, col++);
    const int type_id = sqlite3_column_int(stmt, col++);
    const int level = sqlite3_column_int(stmt, col++);
    const int sensornr = sqlite3_column_int(stmt, col++);

    const Time  obstime   = my_sqlite3_time(stmt, col++);
    const float original  = sqlite3_column_double(stmt, col++);
    const Time  tbtime    = my_sqlite3_time(stmt, col++);
    const float corrected = sqlite3_column_double(stmt, col++);
    const kvalobs::kvControlInfo controlinfo(sqlite3_column_text(stmt, col++));
    const kvalobs::kvUseInfo     useinfo    (sqlite3_column_text(stmt, col++));
    const std::string cfailed = my_sqlite3_string(stmt, col++);
    
    const kvalobs::kvData kvdata(stationid, obstime, original, paramid,
        tbtime, type_id, sensornr, level, corrected, controlinfo, useinfo, cfailed);
    KvalobsData_p kd = boost::make_shared<KvalobsData>(kvdata, false);
    if ((not filter) or filter->accept(kd, true)) {
      //METLIBS_LOG_DEBUG("accepted " << kd->sensorTime());
      data.push_back(kd);
    }
  }

  finalize_statement(stmt, step);

  Q_EMIT newData(request, data);
  Q_EMIT newData(request, ObsData_pv());
}

// ------------------------------------------------------------------------

void SqliteHandler::exec(const std::string& sql)
{
  //METLIBS_LOG_SCOPE(LOGVAL(sql));
  char *zErrMsg = 0;
  int rc = sqlite3_exec(db, sql.c_str(), 0, 0, &zErrMsg);
  if (rc != SQLITE_OK) {
    const std::string what = zErrMsg;
    sqlite3_free(zErrMsg);
    throw std::runtime_error(what);
  }
}

// ------------------------------------------------------------------------

sqlite3_stmt* SqliteHandler::prepare_statement(const std::string& sql)
{
  int status;
  sqlite3_stmt *stmt;
#if SQLITE_VERSION_NUMBER >= 3003009 // see http://www.sqlite.org/oldnews.html
  status = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, 0);
#else
  status = sqlite3_prepare(db, sql.c_str(), sql.length(), &stmt, 0);
#endif
  if (status != SQLITE_OK) {
    std::ostringstream msg;
    msg << "Error preparing SQL statement '" << sql
        << "'; message from sqlite3 is:" << sqlite3_errmsg(db);
    throw std::runtime_error(msg.str());
  }
  return stmt;
}

// ------------------------------------------------------------------------

void SqliteHandler::finalize_statement(sqlite3_stmt* stmt, int lastStep)
{
  sqlite3_finalize(stmt);
  if (lastStep != SQLITE_DONE) {
    std::ostringstream msg;
    msg << "Statement stepping not finished with DONE; error=" << sqlite3_errmsg(db);
    throw std::runtime_error(msg.str());
  }
}

// ========================================================================

SqliteAccess::SqliteAccess(bool useThread)
  : BackgroundAccess(BackgroundHandler_p(new SqliteHandler), useThread)
  , mCountPost(0)
  , mCountDrop(0)
{
  METLIBS_LOG_SCOPE();
  SqliteHandler_p sqlite = boost::static_pointer_cast<SqliteHandler>(handler());
  sqlite->exec("CREATE TABLE data ("
      "stationid   INTEGER NOT NULL, "
      "obstime     TIMESTAMP NOT NULL, "
      "original    FLOAT NOT NULL, "
      "paramid     INTEGER NOT NULL, "
      "tbtime      TIMESTAMP NOT NULL, "
      "typeid      INTEGER NOT NULL, "
      "sensor      CHAR(1) DEFAULT '0', "
      "level       INTEGER DEFAULT 0, "
      "corrected   FLOAT NOT NULL, "
      "controlinfo CHAR(16) DEFAULT '0000000000000000', "
      "useinfo     CHAR(16) DEFAULT '0000000000000000', "
      "cfailed     TEXT DEFAULT NULL);");
}

// ------------------------------------------------------------------------

SqliteAccess::~SqliteAccess()
{
  METLIBS_LOG_SCOPE();
}

// ------------------------------------------------------------------------

void SqliteAccess::postRequest(ObsRequest_p request)
{
  mCountPost += 1;
  BackgroundAccess::postRequest(request);
}

// ------------------------------------------------------------------------

void SqliteAccess::dropRequest(ObsRequest_p request)
{
  mCountDrop += 1;
  BackgroundAccess::dropRequest(request);
}

// ------------------------------------------------------------------------

ObsUpdate_p SqliteAccess::createUpdate(const SensorTime& sensorTime)
{
  return boost::make_shared<KvalobsUpdate>(sensorTime);
}

// ------------------------------------------------------------------------

ObsUpdate_p SqliteAccess::createUpdate(ObsData_p obs)
{
  return boost::make_shared<KvalobsUpdate>(boost::static_pointer_cast<KvalobsData>(obs));
}

// ------------------------------------------------------------------------

bool SqliteAccess::storeUpdates(const ObsUpdate_pv& updates)
{
  const timeutil::ptime tbtime = boost::posix_time::microsec_clock::universal_time();

  kvData_v dataInsert, dataUpdate;
  for (ObsUpdate_pv::const_iterator it = updates.begin(); it != updates.end(); ++it) {
    KvalobsUpdate_p ou = boost::static_pointer_cast<KvalobsUpdate>(*it);
    
    kvalobs::kvData d = ou->data();
    if (ou->changes() & KvalobsUpdate::CHANGED_CORRECTED)
      d.corrected(ou->corrected());
    if (ou->changes() & KvalobsUpdate::CHANGED_CONTROLINFO) {
      d.controlinfo(ou->controlinfo());
      Helpers::updateUseInfo(d);
    }
    if (ou->changes() & KvalobsUpdate::CHANGED_CFAILED)
      d.controlinfo(ou->cfailed());

    if ((ou->changes() & KvalobsUpdate::CHANGED_NEW)) {
      Helpers::updateCfailed(d, "hqc-i");
      // specify tbtime
      d = kvalobs::kvData(d.stationID(), d.obstime(), d.original(),
          d.paramID(), timeutil::to_miTime(tbtime), d.typeID(), d.sensor(), d.level(),
          d.corrected(), d.controlinfo(), d.useinfo(), d.cfailed());
      dataInsert.push_back(d);
    } else {
      Helpers::updateCfailed(d, "hqc-m");
      dataUpdate.push_back(d);
    }
  }

  SqliteHandler_p sqlite = boost::static_pointer_cast<SqliteHandler>(handler());

  ObsData_pv inserted;
  inserted.reserve(dataInsert.size());
  for (kvData_v::const_iterator itI = dataInsert.begin(); itI != dataInsert.end(); ++itI) {
    sqlite->exec(sql4insert(*itI));
    inserted.push_back(boost::make_shared<KvalobsData>(*itI, false));
  }

  ObsData_pv updated;
  updated.reserve(dataUpdate.size());
  for (kvData_v::const_iterator itU = dataUpdate.begin(); itU != dataUpdate.end(); ++itU) {
    sqlite->exec(sql4update(*itU));
    updated.push_back(boost::make_shared<KvalobsData>(*itU, false));
  }

  distributeUpdates(updated, inserted);
  return true;
}

// ------------------------------------------------------------------------

void SqliteAccess::insertDataFromFile(const std::string& filename)
{
  METLIBS_LOG_SCOPE(LOGVAL(filename));
  std::ifstream f(filename.c_str());
  std::string line;

  SqliteHandler_p sqlite = boost::static_pointer_cast<SqliteHandler>(handler());
  const timeutil::ptime tbtime = timeutil::now();

  while (std::getline(f, line)) {
    if (line.empty() or line.at(0) == '#' or line.at(0) == ' ')
      continue;

    std::vector<std::string> columns;
    boost::split(columns, line, boost::is_any_of("\t"));
    if (columns.size() != 7 and columns.size() != 8) {
      //HQC_LOG_WARN("bad line '" << line << "' cols=" << columns.size());
      continue;
    }

    unsigned int c = 0;
    const int stationId = boost::lexical_cast<int>(columns[c++]);
    const int paramId   = boost::lexical_cast<int>(columns[c++]);
    const int typeId    = boost::lexical_cast<int>(columns[c++]);
    const std::string obstime = columns[c++];
    const float original  = boost::lexical_cast<float>(columns[c++]);
    const float corrected = boost::lexical_cast<float>(columns[c++]);
    const std::string controlinfo = columns[c++];
    std::string cfailed;
    if (c<columns.size())
      cfailed = columns[c++];

    const kvalobs::kvControlInfo ci(controlinfo);
    kvalobs::kvUseInfo ui;
    ui.setUseFlags(kvalobs::kvControlInfo(controlinfo));
    const std::string useinfo = ui.flagstring();

    const kvalobs::kvData d(stationId, timeutil::from_iso_extended_string(obstime), original,
        paramId, tbtime, typeId, 0/*sensor*/, 0/*level*/, corrected, ci, ui, cfailed);
    sqlite->exec(sql4insert(d));
  }
}
