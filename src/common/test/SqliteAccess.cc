
#include "SqliteAccess.hh"
#include "SqliteAccessPrivate.hh"

#include "KvalobsData.hh"
#include "KvalobsUpdate.hh"
#include "SimpleBuffer.hh"
#include "QueryTask.hh"

#include "sqlutil.hh"

#include "common/KvHelpers.hh"

#include <puTools/miStringBuilder.h>

#include <boost/algorithm/string.hpp>

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

timeutil::ptime from_iso(const std::string& iso)
{
  if (iso.size() >= 19)
    return timeutil::from_iso_extended_string(iso);
  else
    return timeutil::ptime();
}

std::string to_iso(const timeutil::ptime& t)
{
  if (t.is_not_a_date_time())
    return "NULL";
  else
    return timeutil::to_iso_extended_string(t);
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
      << "'" << d.controlinfo().flagstring() << "', "
      << "'" << d.useinfo().flagstring() << "', "
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

std::string sql4insert(const kvalobs::kvModelData& m)
{
  std::ostringstream sql;
  sql << "INSERT INTO model_data VALUES " << m.toSend();
  return sql.str();
}

struct quoted_bool_t {
public:
  quoted_bool_t(bool v) : value(v) { }
  bool value;
};
std::ostream& operator<<(std::ostream& out, const quoted_bool_t& qb)
{ out << '\'' << (qb.value ? '1' : '0') << '\''; return out; }
quoted_bool_t quoted(bool v) { return quoted_bool_t(v); }

std::string sql4insert(const hqc::hqcObsPgm& op)
{
  std::ostringstream sql;
  sql << "INSERT INTO obs_pgm VALUES "
#if 0
      << op.toSend()
#else
      << "("
      << op.stationID() << ", "
      << op.paramID() << ", "
      << op.level() << ", "
      << op.nr_sensor() << ", "
      << op.typeID() << ", "
      << op.priority_message() << ","
      << quoted(op.collector()) << ","
      << quoted(op.kl00()) << ","
      << quoted(op.kl01()) << ","
      << quoted(op.kl02()) << ","
      << quoted(op.kl03()) << ","
      << quoted(op.kl04()) << ","
      << quoted(op.kl05()) << ","
      << quoted(op.kl06()) << ","
      << quoted(op.kl07()) << ","
      << quoted(op.kl08()) << ","
      << quoted(op.kl09()) << ","
      << quoted(op.kl10()) << ","
      << quoted(op.kl11()) << ","
      << quoted(op.kl12()) << ","
      << quoted(op.kl13()) << ","
      << quoted(op.kl14()) << ","
      << quoted(op.kl15()) << ","
      << quoted(op.kl16()) << ","
      << quoted(op.kl17()) << ","
      << quoted(op.kl18()) << ","
      << quoted(op.kl19()) << ","
      << quoted(op.kl20()) << ","
      << quoted(op.kl21()) << ","
      << quoted(op.kl22()) << ","
      << quoted(op.kl23()) << ","
      << quoted(op.mon())  << ","
      << quoted(op.tue())  << ","
      << quoted(op.wed())  << ","
      << quoted(op.thu())  << ","
      << quoted(op.fri())  << ","
      << quoted(op.sat())  << ","
      << quoted(op.sun())  << ","
      << time2sql(op.fromtime()) << ","
      << time2sql(op.totime()) << ")"
#endif
      ;
  return sql.str();
}

std::string sql4insert(const kvalobs::kvParam& p)
{
  std::ostringstream sql;
  sql << "INSERT INTO param VALUES " << p.toSend();
  return sql.str();
}

std::string sql4insert(const kvalobs::kvStation& s)
{
  std::ostringstream sql;
  sql << "INSERT INTO station VALUES " << s.toSend();
  return sql.str();
}

std::string sql4insert(const kvalobs::kvTypes& t)
{
  std::ostringstream sql;
  sql << "INSERT INTO types VALUES " << t.toSend();
  return sql.str();
}

typedef std::vector<kvalobs::kvData> kvData_v;

class SqliteRow : public ResultRow
{
public:
  SqliteRow(sqlite3_stmt *stmt) : mStmt(stmt) { }

  bool getBool(int index) const override { return sqlite3_column_int(mStmt, index) != 0; }

  int getInt(int index) const override { return sqlite3_column_int(mStmt, index); }

  float getFloat(int index) const override { return sqlite3_column_double(mStmt, index); }

  std::string getStdString(int index) const override { return my_sqlite3_string(mStmt, index); }

  QString getQString(int index) const override { return QString::fromStdString(getStdString(index)); }

  timeutil::ptime getTime(int index) const override { return timeutil::from_iso_extended_string(getStdString(index)); }

private:
  sqlite3_stmt *mStmt;
};

const QString DBVERSION = "d=sqlite:t=kvalobs:v=1";

} // namespace anonymous

// ========================================================================

SqliteQueryRunner::SqliteQueryRunner()
{
  METLIBS_LOG_SCOPE();
  if (sqlite3_open("", &db))
    throw std::runtime_error("could not create sqlite memory db");
}

// ------------------------------------------------------------------------

SqliteQueryRunner::~SqliteQueryRunner()
{
  METLIBS_LOG_SCOPE();
  sqlite3_close(db);
}

// ------------------------------------------------------------------------

QString SqliteQueryRunner::run(QueryTask* qtask)
{
  METLIBS_LOG_SCOPE();

  const std::string sql = qtask->querySql(DBVERSION).toStdString();
  QString message;
  sqlite3_stmt *stmt = prepare_statement(sql, message);
  if (not stmt)
    return message;

  SqliteRow row(stmt);
  int step;
  while ((step = sqlite3_step(stmt)) == SQLITE_ROW)
    qtask->notifyRow(row);
  return finalize_statement(stmt, step, qtask);
}

// ------------------------------------------------------------------------

int SqliteQueryRunner::exec(const std::string& sql)
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

sqlite3_stmt* SqliteQueryRunner::prepare_statement(const std::string& sql, QString& message)
{
  int status;
  sqlite3_stmt *stmt;
#if SQLITE_VERSION_NUMBER >= 3003009 // see http://www.sqlite.org/oldnews.html
  status = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, 0);
#else
  status = sqlite3_prepare(db, sql.c_str(), sql.length(), &stmt, 0);
#endif
  if (status != SQLITE_OK) {
    message = QString("Error preparing SQL statement '%1'; message from sqlite3 is: %2")
        .arg(QString::fromStdString(sql))
        .arg(QString(sqlite3_errmsg(db)));
    HQC_LOG_ERROR(message);
  };
  return stmt;
}

// ------------------------------------------------------------------------

QString SqliteQueryRunner::finalize_statement(sqlite3_stmt* stmt, int lastStep, QueryTask* qtask)
{
  QString status;
  sqlite3_finalize(stmt);
  if (lastStep != SQLITE_DONE) {
    status = QString("Statement stepping not finished with DONE; error=")
        +  sqlite3_errmsg(db);
    HQC_LOG_ERROR(status);
  }
  return status;
}

// ========================================================================

SqliteAccess::SqliteAccess(bool useThread)
  : QueryTaskAccess(std::make_shared<QueryTaskHandler>(std::make_shared<SqliteQueryRunner>(), useThread))
  , mCountPost(0)
  , mCountDrop(0)
{
  METLIBS_LOG_SCOPE();
  SqliteQueryRunner_p sqlite = runner();

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

  sqlite->exec("CREATE TABLE data_history ("
      "version     INTEGER NOT NULL, "
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
      "cfailed     TEXT DEFAULT NULL, "
      "modificationtime TIMESTAMP);");

  sqlite->exec("CREATE TABLE model_data ("
      "stationid   INTEGER NOT NULL, "
      "obstime     TIMESTAMP NOT NULL, "
      "paramid     INTEGER NOT NULL, "
      "level       INTEGER DEFAULT 0, "
      "modelid     INTEGER NOT NULL, "
      "original    FLOAT NOT NULL);");

  std::ostringstream sql;
  sql << "CREATE TABLE obs_pgm ("
      "stationid   INTEGER NOT NULL, "
      "paramid     INTEGER NOT NULL, "
      "level       INTEGER NOT NULL, "
      "nr_sensor   DEFAULT 1, "
      "typeid      INTEGER NOT NULL, "
      "priority_message BOOLEAN DEFAULT TRUE, "
      "collector   BOOLEAN DEFAULT FALSE, ";
  for (int i=0; i<24; ++i)
    sql << "kl" << std::setw(2) << std::setfill('0') << i << " BOOLEAN DEFAULT FALSE, ";
  sql << "mon BOOLEAN DEFAULT FALSE, "
      "tue BOOLEAN DEFAULT FALSE, "
      "wed BOOLEAN DEFAULT FALSE, "
      "thu BOOLEAN DEFAULT FALSE, "
      "fri BOOLEAN DEFAULT FALSE, "
      "sat BOOLEAN DEFAULT FALSE, "
      "sun BOOLEAN DEFAULT FALSE, "
      "fromtime TIMESTAMP NOT NULL, "
      "totime   TIMESTAMP);";
  sqlite->exec(sql.str());

  sqlite->exec("CREATE TABLE param ("
     "paramid     INTEGER NOT NULL, "
     "name        TEXT NOT NULL, "
     "description TEXT, "
     "unit        TEXT, "
     "level_scale INTEGER DEFAULT 0, "
     "comment     TEXT);");

  sqlite->exec("CREATE TABLE station ("
      "stationid     INTEGER NOT NULL, "
      "lat           FLOAT, "
      "lon           FLOAT, "
      "height        FLOAT, "
      "maxspeed      FLOAT, "
      "name          TEXT, "
      "wmonr         INTEGER, "
      "nationalnr    INTEGER, "
      "icaoid        TEXT DEFAULT NULL, "
      "call_sign     TEXT DEFAULT NULL, "
      "stationstr    TEXT, "
      "environmentid INTEGER, "
      "static        BOOLEAN DEFAULT FALSE, "
      "fromtime      TIMESTAMP NOT NULL);");

  sqlite->exec("CREATE TABLE station_param ("
     "stationid     INTEGER   NOT NULL, "
     "paramid       INTEGER   NOT NULL, "
     "level         INTEGER   DEFAULT 0, "
     "sensor        CHARACTER DEFAULT '0', "
     "fromday       INTEGER   NOT NULL, "
     "today         INTEGER   NOT NULL, "
     "hour          INTEGER   DEFAULT (-1), "
     "qcx           TEXT      NOT NULL, "
     "metadata      TEXT, "
     "desc_metadata TEXT, "
     "fromtime      TIMESTAMP NOT NULL);");

  sqlite->exec("CREATE TABLE types ("
      "typeid   INTEGER NOT NULL, "
      "format   TEXT, "
      "earlyobs INTEGER, "
      "lateobs  INTEGER, "
      "read     TEXT, "
      "obspgm   TEXT, "
      "comment  TEXT);");
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
  QueryTaskAccess::postRequest(request);
}

// ------------------------------------------------------------------------

void SqliteAccess::dropRequest(ObsRequest_p request)
{
  mCountDrop += 1;
  QueryTaskAccess::dropRequest(request);
}

// ------------------------------------------------------------------------

ObsUpdate_p SqliteAccess::createUpdate(const SensorTime& sensorTime)
{
  return std::make_shared<KvalobsUpdate>(sensorTime);
}

// ------------------------------------------------------------------------

ObsUpdate_p SqliteAccess::createUpdate(ObsData_p obs)
{
  return std::make_shared<KvalobsUpdate>(std::static_pointer_cast<KvalobsData>(obs));
}

// ------------------------------------------------------------------------

bool SqliteAccess::storeUpdates(const ObsUpdate_pv& updates)
{
  const timeutil::ptime tbtime = boost::posix_time::microsec_clock::universal_time();

  SqliteQueryRunner_p sqlite = runner();

  ObsData_pv updated, inserted;
  for (ObsUpdate_pv::const_iterator it = updates.begin(); it != updates.end(); ++it) {
    KvalobsUpdate_p ou = std::static_pointer_cast<KvalobsUpdate>(*it);

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

  const timeutil::ptime tbtime = timeutil::now();

  while (std::getline(f, line))
    insertDataFromText(line, tbtime);
}

// ------------------------------------------------------------------------

void SqliteAccess::insertDataFromText(const std::string& line, const timeutil::ptime& tbtime)
{
  if (line.empty() or line.at(0) == '#' or line.at(0) == ' ')
    return;
  
  std::vector<std::string> columns;
  boost::split(columns, line, boost::is_any_of("\t"));
  if (columns.size() != 7 and columns.size() != 8) {
    //HQC_LOG_WARN("bad line '" << line << "' cols=" << columns.size());
    return;
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
  ui.setUseFlags(ci);
  
  insertData(kvalobs::kvData(stationId, from_iso(obstime), original,
          paramId, tbtime, typeId, 0/*sensor*/, 0/*level*/, corrected, ci, ui, cfailed));
}

// ------------------------------------------------------------------------

void SqliteAccess::insertData(const kvalobs::kvData& kd)
{
  const std::string sql = sql4insert(kd);
  runner()->exec(sql);
}

// ------------------------------------------------------------------------

void SqliteAccess::dropData(const SensorTime_v& drop)
{
  SqliteQueryRunner_p sqlite = runner();
  for (SensorTime_v::const_iterator it = drop.begin(); it != drop.end(); ++it)
    sqlite->exec(sql4drop(*it));

  distributeUpdates(ObsData_pv(), ObsData_pv(), drop);
}

// ------------------------------------------------------------------------

void SqliteAccess::insertModelFromFile(const std::string& filename)
{
  METLIBS_LOG_SCOPE("loading model data from file '" << filename << "'");
  std::ifstream f(filename.c_str());
  std::string line;

  while (std::getline(f, line)) {
    if (line.empty() or line.at(0) == '#' or line.at(0) == ' ')
      continue;

    try {
      std::vector<std::string> columns;
      boost::split(columns, line, boost::is_any_of("\t"));
      if (columns.size() != 4) {
        HQC_LOG_WARN("bad model line '" << line << "' cols=" << columns.size());
        continue;
      }

      unsigned int c = 0;
      const int stationId = boost::lexical_cast<int>(columns[c++]);
      const int paramId   = boost::lexical_cast<int>(columns[c++]);
      const std::string obstime = columns[c++];
      const float value  = boost::lexical_cast<float>(columns[c++]);

      insertModel(kvalobs::kvModelData(stationId, from_iso(obstime), paramId, 0, 0, value));
    } catch (std::exception& e) {
      HQC_LOG_WARN("error parsing model line '" << line << "' in file '" + filename + "'");
    }
  }
}

// ------------------------------------------------------------------------

void SqliteAccess::insertModel(const kvalobs::kvModelData& kvm)
{
  const std::string sql = sql4insert(kvm);
  runner()->exec(sql);
}

// ------------------------------------------------------------------------

void SqliteAccess::insertStation(const kvalobs::kvStation& kvs)
{
  runner()->exec((miutil::StringBuilder() << "DELETE FROM station WHERE stationid = " << kvs.stationID()).str());
  const std::string sql = sql4insert(kvs);
  runner()->exec(sql);
}

// ------------------------------------------------------------------------

void SqliteAccess::insertObsPgm(const hqc::hqcObsPgm& kvo)
{
  const std::string sql = sql4insert(kvo);
  runner()->exec(sql);
}

// ------------------------------------------------------------------------

void SqliteAccess::insertParam(const kvalobs::kvParam& kvp)
{
  runner()->exec((miutil::StringBuilder() << "DELETE FROM param WHERE paramid = " << kvp.paramID()).str());
  const std::string sql = sql4insert(kvp);
  runner()->exec(sql);
}

// ------------------------------------------------------------------------

void SqliteAccess::insertTypes(const kvalobs::kvTypes& kvt)
{
  runner()->exec((miutil::StringBuilder() << "DELETE FROM types WHERE typeid = " << kvt.typeID()).str());
  const std::string sql = sql4insert(kvt);
  runner()->exec(sql);
}

// ------------------------------------------------------------------------

SqliteQueryRunner_p SqliteAccess::runner()
{
  return std::static_pointer_cast<SqliteQueryRunner>(handler()->runner());
}

// ------------------------------------------------------------------------

void SqliteAccess::execSQL(const std::string& sql)
{
  runner()->exec(sql);
}

// ------------------------------------------------------------------------

void SqliteAccess::clear()
{
  runner()->exec("DELETE FROM data");
  runner()->exec("DELETE FROM model_data");
  runner()->exec("DELETE FROM obs_pgm");
  runner()->exec("DELETE FROM param");
  runner()->exec("DELETE FROM station");
  runner()->exec("DELETE FROM types");
}
