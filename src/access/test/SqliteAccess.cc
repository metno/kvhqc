
#include "SqliteAccess.hh"
#include "SqliteAccessPrivate.hh"

#include "KvalobsData.hh"
#include "SimpleBuffer.hh"

#include <QtCore/QThread>

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

const size_t QUEUE_SIZE = 16;

} // namespace anonymous

// ========================================================================

SqliteHandler::SqliteHandler()
{
  if (sqlite3_open("", &db))
    throw std::runtime_error("could not create sqlite memory db");
}

// ------------------------------------------------------------------------

SqliteHandler::~SqliteHandler()
{
  sqlite3_close(db);
}

// ------------------------------------------------------------------------

ObsData_pv SqliteHandler::queryData(ObsRequest_p request)
{
  METLIBS_LOG_SCOPE();

  const Sensor& sensor = request->sensor();
  const TimeSpan& time = request->timeSpan();
  ObsFilter_p filter   = request->filter();

  ObsData_pv data;
  
  std::ostringstream sql;
  sql << "SELECT d.obstime, d.original, d.tbtime, d.corrected, d.controlinfo, d.useinfo, d.cfailed FROM data d"
      "  WHERE d.stationid = " << sensor.stationId
      << " AND d.paramid = " << sensor.paramId
      << " AND d.typeid = " << sensor.typeId
      << " AND d.level = " << sensor.level
      << " AND d.sensor = " << sensor.sensor
      << " AND d.obstime BETWEEN '" << timeutil::to_iso_extended_string(time.t0()) << "'"
      <<                   " AND '" << timeutil::to_iso_extended_string(time.t1()) << "'";
  if (filter and filter->hasSQL())
    sql << " AND (" << filter->acceptingSQL("d") << ")";
  sql << " ORDER BY d.obstime";
  //METLIBS_LOG_DEBUG(LOGVAL(sql.str()));

  sqlite3_stmt *stmt = prepare_statement(sql.str());

  int step;
  while( (step = sqlite3_step(stmt)) == SQLITE_ROW ) {
    int col = 0;
    const Time  obstime   = my_sqlite3_time(stmt, col++);
    const float original  = sqlite3_column_double(stmt, col++);
    const Time  tbtime    = my_sqlite3_time(stmt, col++);
    const float corrected = sqlite3_column_double(stmt, col++);
    const kvalobs::kvControlInfo controlinfo(sqlite3_column_text(stmt, col++));
    const kvalobs::kvUseInfo     useinfo    (sqlite3_column_text(stmt, col++));
    const std::string cfailed = my_sqlite3_string(stmt, col++);
    
    const kvalobs::kvData kvdata(sensor.stationId, obstime, original, sensor.paramId,
        tbtime, sensor.typeId, sensor.sensor, sensor.level,
        corrected, controlinfo, useinfo, cfailed);
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

// based on Qt4 blocking fortune client example

SqliteThread::SqliteThread(SqliteHandler_p sqlh)
  : mSqlite(sqlh)
  , mDone(false)
{
}

SqliteThread::~SqliteThread()
{
  mMutex.lock();
  mDone = true;
  mCondition.wakeOne();
  mMutex.unlock();
  wait();
}

void SqliteThread::enqueueRequest(ObsRequest_p request)
{
  QMutexLocker locker(&mMutex);
  mQueue.push(QueuedQuery(1, request));
  
  if (!isRunning())
    start();
  else
    mCondition.wakeOne();
}

void SqliteThread::run()
{
  while (!mDone) {
    ObsRequest_p request;

    mMutex.lock();
    if (mQueue.empty())
      mCondition.wait(&mMutex);
    if (not mQueue.empty()) {
      request = mQueue.top().request;
      mQueue.pop();
    }
    mMutex.unlock();

    if (request) {
      ObsData_pv data = mSqlite->queryData(request);
      // TODO why lock before emit in example?
      Q_EMIT newData(request, data);
    }
  }
}

// ========================================================================

SqliteAccess::SqliteAccess(bool useThread)
  : mSqlite(new SqliteHandler)
  , mCountPost(0)
  , mCountDrop(0)
  , mThread(0)
{
  mSqlite->exec("CREATE TABLE data ("
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

  if (useThread) {
    mThread = new SqliteThread(mSqlite);
    connect(mThread, SIGNAL(newData(ObsRequest_p, const ObsData_pv&)),
        this, SLOT(onNewData(ObsRequest_p, const ObsData_pv&)));
  }
}

// ------------------------------------------------------------------------

SqliteAccess::~SqliteAccess()
{
  METLIBS_LOG_SCOPE();
  delete mThread;
}

// ------------------------------------------------------------------------

void SqliteAccess::checkUpdates()
{
  METLIBS_LOG_SCOPE();
  for (ObsRequest_pv::iterator it = mRequests.begin(); it != mRequests.end(); ++it) {
    ObsRequest_p request = *it;
    request->updateData(ObsData_pv()); // FIXME do reasonable update check
  }
}

// ------------------------------------------------------------------------

void SqliteAccess::resubscribe()
{
  METLIBS_LOG_SCOPE();
}

// ------------------------------------------------------------------------

void SqliteAccess::postRequest(ObsRequest_p request)
{
  METLIBS_LOG_SCOPE();
  mCountPost += 1;

  mRequests.push_back(request);
  {
    const int stationId = request->sensor().stationId;
    stationid_count_m::iterator it = mStationCounts.find(stationId);
    if (it != mStationCounts.end()) {
      it->second += 1;
    } else {
      mStationCounts.insert(std::make_pair(stationId, 1));
      resubscribe();
    }
  }

  if (mThread)
    mThread->enqueueRequest(request);
  else
    onNewData(request, mSqlite->queryData(request));

  //checkUpdates();
}

// ------------------------------------------------------------------------

void SqliteAccess::onNewData(ObsRequest_p request, const ObsData_pv& data)
{
  METLIBS_LOG_SCOPE();

  ObsRequest_pv::iterator it = std::find(mRequests.begin(), mRequests.end(), request);
  if (it != mRequests.end()) {
    METLIBS_LOG_DEBUG(LOGVAL(request->sensor()) << LOGVAL(request->timeSpan()));
    request->newData(data); // FIXME this is not exception safe
    request->completed(false);
  } else {
    METLIBS_LOG_DEBUG("request has been dropped, do nothing");
  }
}

// ------------------------------------------------------------------------

void SqliteAccess::dropRequest(ObsRequest_p request)
{
  METLIBS_LOG_SCOPE();
  mCountDrop += 1;

  // TODO what to do with requests that are processed in bg thread?
  ObsRequest_pv::iterator it = std::find(mRequests.begin(), mRequests.end(), request);
  if (it == mRequests.end()) {
    METLIBS_LOG_ERROR("dropping unknown request");
    return;
  }

  mRequests.erase(it);
  //if (mThread) // TODO
  //  mThread->unqueueRequest(request);

  const int stationId = request->sensor().stationId;
  stationid_count_m::iterator itC = mStationCounts.find(stationId);
  if (itC == mStationCounts.end() or itC->second == 0) {
    METLIBS_LOG_ERROR("dropping request with zero station count");
    return;
  }

  itC->second -= 1;
  if (itC->second == 0) {
    mStationCounts.erase(stationId);
    resubscribe();
  }
}

// ------------------------------------------------------------------------

ObsUpdate_p SqliteAccess::createUpdate(const SensorTime& sensorTime)
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
    mSqlite->exec(sql.str());
  }
}

