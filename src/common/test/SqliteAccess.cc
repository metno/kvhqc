
#include "SqliteAccess.hh"
#include "SqliteAccessPrivate.hh"

#include "KvalobsData.hh"
#include "SimpleBuffer.hh"

#include "sqlutil.hh"

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

ObsData_pv SqliteHandler::queryData(ObsRequest_p request)
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

  return data;
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
  return ObsUpdate_p();
}

// ------------------------------------------------------------------------

ObsUpdate_p SqliteAccess::createUpdate(ObsData_p obs)
{
  return ObsUpdate_p();
}

// ------------------------------------------------------------------------

bool SqliteAccess::storeUpdates(const ObsUpdate_pv& updates)
{
  return false;
}

// ------------------------------------------------------------------------

void SqliteAccess::insertDataFromFile(const std::string& filename)
{
  METLIBS_LOG_SCOPE(LOGVAL(filename));
  std::ifstream f(filename.c_str());
  std::string line;

  SqliteHandler_p sqlite = boost::static_pointer_cast<SqliteHandler>(handler());
  const std::string tbtime = timeutil::to_iso_extended_string(timeutil::now());

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

    kvalobs::kvUseInfo ui;
    ui.setUseFlags(kvalobs::kvControlInfo(controlinfo));
    const std::string useinfo = ui.flagstring();

    std::ostringstream sql;
    sql << "INSERT INTO data VALUES("
        << stationId << ", "
        << "'" << obstime << "', "
        << original << ", "
        << paramId << ", "
        << "'" << tbtime << "', "
        << typeId << ", "
        << "'0', 0, " // sensor + level
        << corrected << ", "
        << "'" << controlinfo << "', "
        << "'" << useinfo << "', "
        << "'" << cfailed << "')";
    sqlite->exec(sql.str());
  }
}
