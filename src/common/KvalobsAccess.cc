
#include "KvalobsAccess.hh"
#include "KvalobsAccessPrivate.hh"

#include "KvalobsData.hh"
#include "KvalobsUpdate.hh"
#include "sqlutil.hh"
#include "common/Functors.hh"
#include "common/KvHelpers.hh"
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

const size_t QUERY_DATA_CHUNKSIZE = 32;

} // namespace anonymous

// ========================================================================

void KvalobsHandler::initialize()
{
  mKvalobsDB = hqcApp->kvalobsDB(QDBNAME);
}

// ------------------------------------------------------------------------

void KvalobsHandler::finalize()
{
  METLIBS_LOG_SCOPE();
  mKvalobsDB.close();
  QSqlDatabase::removeDatabase(QDBNAME);
}

// ------------------------------------------------------------------------

void KvalobsHandler::queryData(ObsRequest_p request)
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
    sql << " AND (" << filter->acceptingSQL("d.") << ")";
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
      const float original  = query.value(col++).toFloat();
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
        if (data.size() >= QUERY_DATA_CHUNKSIZE) {
          Q_EMIT newData(request, data);
          data.clear();
        }
      }
    }
    if (not data.empty())
      Q_EMIT newData(request, data);
    Q_EMIT newData(request, ObsData_pv());
  } else {
    HQC_LOG_ERROR("query '" << sql << "' failed: " << query.lastError().text());
  }
}

// ========================================================================

KvalobsAccess::KvalobsAccess()
  : BackgroundAccess(BackgroundHandler_p(new KvalobsHandler), true)
  , mDataReinserter(0)
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
    for (Sensor_s::const_iterator itS = sensors.begin(); itS != sensors.end(); ++itS)
      ul->addStation(itS->stationId);
  }
}

// ------------------------------------------------------------------------

void KvalobsAccess::checkUnsubscribe(const Sensor_s& sensors)
{
  if (AbstractUpdateListener* ul = updateListener()) {
    for (Sensor_s::const_iterator itS = sensors.begin(); itS != sensors.end(); ++itS)
      ul->removeStation(itS->stationId);
  }
}

// ------------------------------------------------------------------------

void KvalobsAccess::onUpdated(const kvData_v& data)
{
  METLIBS_LOG_SCOPE();
  ObsData_pv updated;
  updated.reserve(data.size());
  for (kvData_v::const_iterator it=data.begin(); it!=data.end(); ++it)
    updated.push_back(boost::make_shared<KvalobsData>(*it, false));
  distributeUpdates(updated, ObsData_pv(), SensorTime_v());
}

// ------------------------------------------------------------------------

ObsUpdate_p KvalobsAccess::createUpdate(ObsData_p obs)
{
  return boost::make_shared<KvalobsUpdate>(boost::static_pointer_cast<KvalobsData>(obs));
}

// ------------------------------------------------------------------------

ObsUpdate_p KvalobsAccess::createUpdate(const SensorTime& sensorTime)
{
  return boost::make_shared<KvalobsUpdate>(sensorTime);
}

// ------------------------------------------------------------------------

bool KvalobsAccess::storeUpdates(const ObsUpdate_pv& updates)
{
  METLIBS_LOG_SCOPE();
  if (not hasReinserter()) {
    METLIBS_LOG_DEBUG("not authorized");
    return false;
  }

  METLIBS_LOG_DEBUG(updates.size() << " updates");
  if (updates.empty())
    return true;

  std::list<kvalobs::kvData> store;
  ObsData_pv modifiedObs, createdObs;
  const timeutil::ptime tbtime = boost::posix_time::microsec_clock::universal_time();
  for (ObsUpdate_pv::const_iterator it = updates.begin(); it != updates.end(); ++it) {
    KvalobsUpdate_p ou = boost::static_pointer_cast<KvalobsUpdate>(*it);

    KvalobsData_p d;
    if (ou->obs()) {
      d = Helpers::modifiedData(ou->obs(), ou->corrected(), ou->controlinfo(), ou->cfailed());
      Helpers::updateCfailed(d->data(), "hqc-m");
      modifiedObs.push_back(d);
    } else {
      d = Helpers::createdData(ou->sensorTime(), tbtime, ou->corrected(), ou->controlinfo(), ou->cfailed());
      Helpers::updateCfailed(d->data(), "hqc-i");
      createdObs.push_back(d);
    }

    store.push_back(d->data());
  }
  METLIBS_LOG_DEBUG(store.size() << " to store");

  CKvalObs::CDataSource::Result_var res = mDataReinserter->insert(store);
  if (res->res == CKvalObs::CDataSource::OK) {
    distributeUpdates(modifiedObs, createdObs, SensorTime_v());
    return true;
  } else {
    std::ostringstream msg;
    msg << "saving these failed: modified:";
    for (ObsData_pv::const_iterator it = modifiedObs.begin(); it != modifiedObs.end(); ++it)
      msg << ' ' << (*it)->sensorTime();
    msg << "; created:";
    for (ObsData_pv::const_iterator it = createdObs.begin(); it != createdObs.end(); ++it)
        msg << ' ' << (*it)->sensorTime();
    HQC_LOG_WARN(msg.str());
    return false;
  }
}
