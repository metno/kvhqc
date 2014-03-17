
#include "KvalobsAccess.hh"
#include "KvalobsAccessPrivate.hh"

#include "KvalobsData.hh"
#include "sqlutil.hh"
#include "common/HqcApplication.hh"

#include <QtCore/QVariant>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

#include <boost/algorithm/string.hpp>
#include <boost/make_shared.hpp>

#define MILOGGER_CATEGORY "kvhqc.KvalobsAccess"
#include "common/ObsLogging.hh"

namespace /*anonymous*/ {

std::string my_qsql_string(const QVariant& v)
{
  return v.toString().toStdString();
}

Time my_qsql_time(const QVariant& v)
{
  return timeutil::from_iso_extended_string(my_qsql_string(v));
}

const QString QDBNAME = "kvalobs_bg";

} // namespace anonymous

// ========================================================================

void KvalobsHandler::initialize()
{
  mKvalobsDB = hqcApp->kvalobsDB(QDBNAME);
}

// ------------------------------------------------------------------------

void KvalobsHandler::finalize()
{
  QSqlDatabase::removeDatabase(QDBNAME);
}

// ------------------------------------------------------------------------

ObsData_pv KvalobsHandler::queryData(ObsRequest_p request)
{
  METLIBS_LOG_SCOPE();

  QSqlQuery query(mKvalobsDB);

  const Sensor_s& sensors = request->sensors();
  const TimeSpan& time = request->timeSpan();
  ObsFilter_p filter   = request->filter();

  std::ostringstream sql;
  sql << "SELECT d.stationid, d.paramid, d.typeid, d.level, d.sensor,"
      " d.obstime, d.original, d.tbtime, d.corrected, d.controlinfo, d.useinfo, d.cfailed"
      " FROM data d WHERE ";
  sensors2sql(sql, sensors, "d.");
  sql << " AND d.obstime BETWEEN " << time2sql(time.t0()) << " AND " << time2sql(time.t1());
  if (filter and filter->hasSQL())
    sql << " AND (" << filter->acceptingSQL("d") << ")";
  sql << " ORDER BY d.stationid, d.paramid, d.typeid, d.level, d.sensor, d.obstime";
  //METLIBS_LOG_DEBUG(LOGVAL(sql.str()));


  ObsData_pv data;
  if (query.exec(QString::fromStdString(sql.str()))) {
    while (query.next()) {
      int col = 0;
      
      const int stationid = query.value(col++).toInt();
      const int paramid   = query.value(col++).toInt();
      const int type_id   = query.value(col++).toInt();
      const int level     = query.value(col++).toInt();
      const int sensornr  = query.value(col++).toInt();
      
      const Time  obstime   = my_qsql_time(query.value(col++));
      const float original  = query.value(col++).toInt();
      const Time  tbtime    = my_qsql_time(query.value(col++));
      const float corrected = query.value(col++).toFloat();;
      const kvalobs::kvControlInfo controlinfo(my_qsql_string(query.value(col++)));
      const kvalobs::kvUseInfo     useinfo    (my_qsql_string(query.value(col++)));
      const std::string cfailed = my_qsql_string(query.value(col++));
      
      const kvalobs::kvData kvdata(stationid, obstime, original, paramid,
          tbtime, type_id, sensornr, level, corrected, controlinfo, useinfo, cfailed);
      KvalobsData_p kd = boost::make_shared<KvalobsData>(kvdata, false);
      if ((not filter) or filter->accept(kd, true)) {
        //METLIBS_LOG_DEBUG("accepted " << kd->sensorTime());
        data.push_back(kd);
      }
    }
  } else {
    HQC_LOG_ERROR("query '" << sql << "' failed: " << query.lastError().text());
  }

  return data;
}

// ========================================================================

KvalobsAccess::KvalobsAccess()
  : BackgroundAccess(BackgroundHandler_p(new KvalobsHandler), true)
{
  if (AbstractUpdateListener* ul = updateListener())
    connect(ul, SIGNAL(updated(const kvData_v&)), this, SLOT(onUpdated(const kvData_v&)));
  else
    HQC_LOG_WARN("no UpdateListener");
}

// ------------------------------------------------------------------------

KvalobsAccess::~KvalobsAccess()
{
  // TODO unsubscribe all
}

// ------------------------------------------------------------------------

void KvalobsAccess::postRequest(ObsRequest_p request)
{
  METLIBS_LOG_SCOPE();
  checkSubscribe(request->sensors());
  BackgroundAccess::postRequest(request);
}

// ------------------------------------------------------------------------

void KvalobsAccess::dropRequest(ObsRequest_p request)
{
  METLIBS_LOG_SCOPE();
  BackgroundAccess::dropRequest(request);
  checkUnsubscribe(request->sensors());
}

// ------------------------------------------------------------------------

void KvalobsAccess::checkSubscribe(const Sensor_s& sensors)
{
  if (AbstractUpdateListener* ul = updateListener()) {
    for (Sensor_s::const_iterator itS = sensors.begin(); itS != sensors.end(); ++itS) {
      const int stationId = itS->stationId;
      ul->addStation(stationId);
    }
  }
}

// ------------------------------------------------------------------------

void KvalobsAccess::checkUnsubscribe(const Sensor_s& sensors)
{
  if (AbstractUpdateListener* ul = updateListener()) {
    for (Sensor_s::const_iterator itS = sensors.begin(); itS != sensors.end(); ++itS) {
      const int stationId = itS->stationId;
      ul->removeStation(stationId);
    }
  }
}

// ------------------------------------------------------------------------

void KvalobsAccess::onUpdated(const kvData_v&)
{
  METLIBS_LOG_SCOPE();
}

// ------------------------------------------------------------------------

ObsUpdate_p KvalobsAccess::createUpdate(const SensorTime& sensorTime)
{
  return ObsUpdate_p();
}

// ------------------------------------------------------------------------

bool KvalobsAccess::storeUpdates(const ObsUpdate_pv& updates)
{
  return false;
}
