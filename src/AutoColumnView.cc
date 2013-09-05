
#include "AutoColumnView.hh"

#include "Helpers.hh"
#include "HqcApplication.hh"

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>

#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.AutoColumnView"
#include "HqcLogging.hh"

namespace {
const char CHANGES_TABLE[] = "view_changes";
const char CHANGES_TABLE_CREATE[] = "CREATE TABLE view_changes ("
    "stationid    INTEGER NOT NULL,"
    "paramid      INTEGER NOT NULL,"
    "view_type    TEXT    NOT NULL,"
    "view_id      TEXT    NOT NULL,"
    "view_changes TEXT    NOT NULL"
    ");";
const char CHANGES_TABLE_INSERT[] = "INSERT INTO view_changes VALUES ("
    ":stationid, :paramid, :view_type, :view_id, :view_changes"
    ");";
const char CHANGES_TABLE_SELECT[] = "SELECT view_changes FROM view_changes"
    " WHERE stationid = :stationid AND paramid = :paramid AND view_type = :view_type AND view_id = :view_id;";
}

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

  storeViewChanges();
  mSensorTime = st;
  replayViewChanges();
}

void AutoColumnView::storeViewChanges()
{
  METLIBS_LOG_SCOPE();
  QSqlDatabase db = hqcApp->configDB();
  if (not db.tables().contains(CHANGES_TABLE))
    db.exec(CHANGES_TABLE_CREATE);

  if (mSensorTime.valid()) {
    // record changes
    db.transaction();
    QSqlQuery insert(db);
    insert.prepare(CHANGES_TABLE_INSERT);
    BOOST_FOREACH(ViewInfo& vi, mViews) {
      insert.bindValue("stationid", mSensorTime.sensor.stationId);
      insert.bindValue("paramid",   mSensorTime.sensor.paramId);
      insert.bindValue("view_type", QString::fromStdString(vi.view->type()));
      insert.bindValue("view_id",   QString::fromStdString(vi.view->id()));
      insert.bindValue("view_changes", QString::fromStdString(vi.view->changes()));
      insert.exec();
      insert.finish();
    }
    db.commit();
  }
}

void AutoColumnView::replayViewChanges()
{
  METLIBS_LOG_SCOPE();

  const Sensors_t defSens = defaultSensors();
  const TimeRange defLimits = defaultTimeLimits();

  QSqlQuery query(hqcApp->configDB());
  query.prepare(CHANGES_TABLE_SELECT);
  query.bindValue("stationid", mSensorTime.sensor.stationId);
  query.bindValue("paramid",   mSensorTime.sensor.paramId);
  BOOST_FOREACH(ViewInfo& vi, mViews) {
    vi.view->setSensorsAndTimes(defSens, defLimits);

    METLIBS_LOG_DEBUG(LOGVAL(vi.view->type()) << LOGVAL(vi.view->id()));
    query.bindValue("view_type", QString::fromStdString(vi.view->type()));
    query.bindValue("view_id",   QString::fromStdString(vi.view->id()));
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
