
#include "ViewChanges.hh"

#include "common/KvHelpers.hh"
#include "common/Sensor.hh"
#include "common/HqcApplication.hh"
#include "util/stringutil.hh"

#include <QtCore/QVariant>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.ViewChanges"
#include "common/ObsLogging.hh"

namespace /*anonymous*/ {

const char CHANGES_TABLE[] = "user_view_changes";
const char CHANGES_TABLE_CREATE[] = "CREATE TABLE user_view_changes ("
    "stationid    INTEGER NOT NULL,"
    "paramid      INTEGER NOT NULL,"
    "view_type    TEXT    NOT NULL,"
    "view_id      TEXT    NOT NULL,"
    "view_changes TEXT    NOT NULL"
    ");";

const char CHANGES_TABLE_UPDATE[] = "UPDATE user_view_changes SET view_changes = :vchanges"
    " WHERE stationid = :sid AND paramid = :pid AND view_type = :vtype AND view_id = :vid;";
const char CHANGES_TABLE_INSERT[] = "INSERT INTO user_view_changes VALUES"
    " (:sid, :pid, :vtype, :vid, :vchanges)";

const char CHANGES_TABLE_SELECT[] = "SELECT view_changes FROM user_view_changes"
    " WHERE stationid = :sid AND paramid = :pid AND view_type = :vtype AND view_id = :vid;";

} // anonymous namespace


void ViewChanges::store(const Sensor& s, const std::string& vtype, const std::string& vid, const std::string& vchanges)
{
  METLIBS_LOG_SCOPE();
  METLIBS_LOG_DEBUG(LOGVAL(s) << LOGVAL(vtype) << LOGVAL(vid) << LOGVAL(vchanges));

  if (not hqcApp)
    return;

  const QString qtype = Helpers::fromUtf8(vtype), qid = Helpers::fromUtf8(vid), qchanges = Helpers::fromUtf8(vchanges);

  QSqlDatabase db = hqcApp->configDB();
  if (not db.tables().contains(CHANGES_TABLE))
    db.exec(CHANGES_TABLE_CREATE);

  db.transaction();

  QSqlQuery update(db);
  update.prepare(CHANGES_TABLE_UPDATE);
  update.bindValue(":sid", s.stationId);
  update.bindValue(":pid", s.paramId);
  update.bindValue(":vtype",    qtype);
  update.bindValue(":vid",      qid);
  update.bindValue(":vchanges", qchanges);

  if (not update.exec())
    HQC_LOG_ERROR("error while updating: " << update.lastError().text());
  const int nup = update.numRowsAffected();
  update.finish();
  
  if (nup == 0) {
    QSqlQuery insert(db);
    insert.prepare(CHANGES_TABLE_INSERT);
    insert.bindValue(":sid", s.stationId);
    insert.bindValue(":pid", s.paramId);
    insert.bindValue(":vtype",    qtype);
    insert.bindValue(":vid",      qid);
    insert.bindValue(":vchanges", qchanges);
    if (not insert.exec())
      HQC_LOG_ERROR("error while inserting: " << insert.lastError().text());
    insert.finish();
  }
  db.commit();
}

std::string ViewChanges::fetch(const Sensor& s, const std::string& vtype, const std::string& vid)
{
  METLIBS_LOG_SCOPE();
  METLIBS_LOG_DEBUG(LOGVAL(s) << LOGVAL(vtype) << LOGVAL(vid));

  if (hqcApp) {
    QSqlQuery query(hqcApp->configDB());
    query.prepare(CHANGES_TABLE_SELECT);
    query.bindValue(":sid", s.stationId);
    query.bindValue(":pid", s.paramId);
    query.bindValue(":vtype", QString::fromStdString(vtype));
    query.bindValue(":vid",   QString::fromStdString(vid));
    if (not query.exec())
      HQC_LOG_ERROR("error while querying: " << query.lastError().text());
    else if (query.next()) {
      const std::string vchanges = query.value(0).toString().toStdString();
      METLIBS_LOG_DEBUG(LOGVAL(vchanges));
      return vchanges;
    }
  }
  return "";
}

TimeRange ViewChanges::defaultTimeLimits(const SensorTime& st)
{
  int hours = 24;
  if (st.sensor.paramId == kvalobs::PARAMID_RR_24)
    hours = 7*24;
  
  const boost::posix_time::time_duration dt = boost::posix_time::hours(hours);
  const timeutil::ptime& t = st.time;
  return TimeRange(t - dt, t + dt);
}
