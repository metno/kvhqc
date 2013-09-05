
#include "AutoColumnView.hh"

#include "Helpers.hh"
#include "HqcApplication.hh"

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.AutoColumnView"
#include "HqcLogging.hh"

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

AutoColumnView::AutoColumnView()
{
}

AutoColumnView::~AutoColumnView()
{
}

void AutoColumnView::navigateTo(const SensorTime& st)
{
  METLIBS_LOG_SCOPE();
  if (eq_SensorTime()(mSensorTime, st))
    return;

  if (mSensorTime.valid())
    storeViewChanges();

  mSensorTime = st;

  if (mSensorTime.valid())
    replayViewChanges();
}

void AutoColumnView::storeViewChanges()
{
  METLIBS_LOG_SCOPE();

  QSqlDatabase db = hqcApp->configDB();
  if (not db.tables().contains(CHANGES_TABLE))
    db.exec(CHANGES_TABLE_CREATE);

  db.transaction();
  QSqlQuery insert(db), update(db);
  insert.prepare(CHANGES_TABLE_INSERT);
  update.prepare(CHANGES_TABLE_UPDATE);

  const Sensor& s = mSensorTime.sensor;
  update.bindValue(":sid", s.stationId);
  update.bindValue(":pid", s.paramId);
  insert.bindValue(":sid", s.stationId);
  insert.bindValue(":pid", s.paramId);
  BOOST_FOREACH(ViewInfo& vi, mViews) {
    const std::string vtype = vi.view->type(), vid = vi.view->id(), vchanges = vi.view->changes();
    METLIBS_LOG_DEBUG(LOGVAL(vtype) << LOGVAL(vid) << LOGVAL(vchanges));
    const QString qtype = QString::fromStdString(vtype), qid = QString::fromStdString(vid), qchanges = QString::fromStdString(vchanges);

    update.bindValue(":vtype",    qtype);
    update.bindValue(":vid",      qid);
    update.bindValue(":vchanges", qchanges);
    if (not update.exec())
      METLIBS_LOG_ERROR("error while updating: " << update.lastError().text());
    const int nup = update.numRowsAffected();
    update.finish();

    if (nup == 0) {
      insert.bindValue(":vtype",    qtype);
      insert.bindValue(":vid",      qid);
      insert.bindValue(":vchanges", qchanges);
      if (not insert.exec())
        METLIBS_LOG_ERROR("error while inserting: " << insert.lastError().text());
      insert.finish();
    }
  }
  db.commit();
}

void AutoColumnView::replayViewChanges()
{
  METLIBS_LOG_SCOPE();

  const Sensors_t defSens = defaultSensors();
  const TimeRange defLimits = defaultTimeLimits();

  QSqlQuery query(hqcApp->configDB());
  query.prepare(CHANGES_TABLE_SELECT);
  query.bindValue(":sid", mSensorTime.sensor.stationId);
  query.bindValue(":pid", mSensorTime.sensor.paramId);
  BOOST_FOREACH(ViewInfo& vi, mViews) {
    vi.view->setSensorsAndTimes(defSens, defLimits);

    METLIBS_LOG_DEBUG(LOGVAL(vi.view->type()) << LOGVAL(vi.view->id()));
    query.bindValue(":vtype", QString::fromStdString(vi.view->type()));
    query.bindValue(":vid",   QString::fromStdString(vi.view->id()));
    query.exec();
    if (query.next()) {
      const std::string vchanges = query.value(0).toString().toStdString();
      METLIBS_LOG_DEBUG(LOGVAL(vchanges));
      vi.view->replay(vchanges);
    }
    query.finish();

    vi.view->navigateTo(mSensorTime);
  }
}

void AutoColumnView::attachView(ViewP v)
{
    mViews.push_back(ViewInfo(v));
    if (mSensorTime.valid())
        v->setSensorsAndTimes(defaultSensors(), defaultTimeLimits());
}

void AutoColumnView::detachView(ViewP v)
{
    for(Views_t::iterator it = mViews.begin(); it != mViews.end(); ++it) {
        if (it->view == v) {
            mViews.erase(it);
            return;
        }
    }
    METLIBS_LOG_WARN("cannot detach view");
}

AutoColumnView::Sensors_t AutoColumnView::defaultSensors()
{
  return Helpers::relatedSensors(mSensorTime);
}

TimeRange AutoColumnView::defaultTimeLimits()
{
    int hours = 24;
    if (mSensorTime.sensor.paramId == kvalobs::PARAMID_RR_24)
        hours = 7*24;

    const boost::posix_time::time_duration dt = boost::posix_time::hours(hours);
    const timeutil::ptime& t = mSensorTime.time;
    return TimeRange(t - dt, t + dt);
}
