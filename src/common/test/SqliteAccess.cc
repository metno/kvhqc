
#include "SqliteAccess.hh"
#include "SqliteAccessPrivate.hh"

#include "KvalobsData.hh"
#include "KvalobsUpdate.hh"
#include "SimpleBuffer.hh"
#include "QueryTask.hh"

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

std::string sql4drop(const SensorTime& st)
{
  const Sensor& s = st.sensor;
  std::ostringstream sql;
  sql << "DELETE FROM data "
      << "WHERE stationid = " << s.stationId
      << "  AND obstime = " << time2sql(st.time)
      << "  AND paramid = " << s.paramId
      << "  AND typeid = " << s.typeId
      << "  AND sensor = '" << s.sensor << "'"
      << "  AND level = " << s.level;
  return sql.str();
}

typedef std::vector<kvalobs::kvData> kvData_v;

class SqliteRow : public ResultRow
{
public:
  SqliteRow(sqlite3_stmt *stmt) : mStmt(stmt) { }

  int getInt(int index) const
    { return sqlite3_column_int(mStmt, index); }

  float getFloat(int index) const
    { return sqlite3_column_double(mStmt, index); }

  std::string getStdString(int index) const
    { return my_sqlite3_string(mStmt, index); }

  QString getQString(int index) const
    { return QString::fromStdString(getStdString(index)); }

private:
  sqlite3_stmt *mStmt;
};

const QString DBVERSION = "test_sqlite:1";

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

void SqliteHandler::queryTask(QueryTask* qtask)
{
  METLIBS_LOG_SCOPE();

  sqlite3_stmt *stmt = prepare_statement(qtask->querySql(DBVERSION).toStdString(), qtask);
  if (not stmt)
    return;

  SqliteRow row(stmt);
  int step;
  while ((step = sqlite3_step(stmt)) == SQLITE_ROW)
    qtask->notifyRow(row);
  finalize_statement(stmt, step, qtask);
}

// ------------------------------------------------------------------------

int SqliteHandler::exec(const std::string& sql)
{
  //METLIBS_LOG_SCOPE(LOGVAL(sql));
  char *zErrMsg = 0;
  int rc = sqlite3_exec(db, sql.c_str(), 0, 0, &zErrMsg);
  if (rc != SQLITE_OK) {
    const std::string what = zErrMsg;
    sqlite3_free(zErrMsg);
    throw std::runtime_error(what);
  }
  return sqlite3_changes(db);
}

// ------------------------------------------------------------------------

sqlite3_stmt* SqliteHandler::prepare_statement(const std::string& sql, QueryTask* qtask)
{
  int status;
  sqlite3_stmt *stmt;
#if SQLITE_VERSION_NUMBER >= 3003009 // see http://www.sqlite.org/oldnews.html
  status = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, 0);
#else
  status = sqlite3_prepare(db, sql.c_str(), sql.length(), &stmt, 0);
#endif
  if (status != SQLITE_OK) {
    QString message = QString("Error preparing SQL statement '%1'; message from sqlite3 is: %2")
        .arg(QString::fromStdString(sql))
        .arg(QString(sqlite3_errmsg(db)));
    qtask->notifyError(message);
  }
  return stmt;
}

// ------------------------------------------------------------------------

void SqliteHandler::finalize_statement(sqlite3_stmt* stmt, int lastStep, QueryTask* qtask)
{
  sqlite3_finalize(stmt);
  if (lastStep == SQLITE_DONE) {
    qtask->notifyDone();
  } else {
    QString message = QString("Statement stepping not finished with DONE; error=")
        +  sqlite3_errmsg(db);
    qtask->notifyError(message);
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

  SqliteHandler_p sqlite = boost::static_pointer_cast<SqliteHandler>(handler());

  ObsData_pv updated, inserted;
  for (ObsUpdate_pv::const_iterator it = updates.begin(); it != updates.end(); ++it) {
    KvalobsUpdate_p ou = boost::static_pointer_cast<KvalobsUpdate>(*it);

    KvalobsData_p d;
    if (ou->obs()) {
      d = Helpers::modifiedData(ou->obs(), ou->corrected(), ou->controlinfo(), ou->cfailed());
      Helpers::updateCfailed(d->data(), "hqc-m");
      sqlite->exec(sql4update(d->data()));
      updated.push_back(d);
    } else {
      d = Helpers::createdData(ou->sensorTime(), tbtime, ou->corrected(), ou->controlinfo(), ou->cfailed());
      Helpers::updateCfailed(d->data(), "hqc-i");
      sqlite->exec(sql4insert(d->data()));
      inserted.push_back(d);
    }
  }

  distributeUpdates(updated, inserted, SensorTime_v());
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

// ------------------------------------------------------------------------

void SqliteAccess::dropData(const SensorTime_v& drop)
{
  SqliteHandler_p sqlite = boost::static_pointer_cast<SqliteHandler>(handler());
  for (SensorTime_v::const_iterator it = drop.begin(); it != drop.end(); ++it)
    sqlite->exec(sql4drop(*it));

  distributeUpdates(ObsData_pv(), ObsData_pv(), drop);
}
