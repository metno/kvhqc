
#include "KvalobsAccess.hh"
#include "KvalobsAccessPrivate.hh"

#include "KvalobsData.hh"
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
      }
    }
  } else {
    HQC_LOG_ERROR("query '" << sql << "' failed: " << query.lastError().text());
  }

  return data;
}

// ========================================================================

KvalobsUpdate::KvalobsUpdate(KvalobsData_p kvdata)
  : mSensorTime(kvdata->sensorTime())
  , mData(kvdata->data())
  , mChanged(0)
{
}

// ------------------------------------------------------------------------

KvalobsUpdate::KvalobsUpdate(const SensorTime& st)
  : mSensorTime(st)
  , mData(Helpers::getMissingKvData(st))
  , mChanged(CHANGED_NEW)
{
}

// ------------------------------------------------------------------------

void KvalobsUpdate::setCorrected(float c)
{
  if (Helpers::float_eq()(c, mData.corrected()))
    mChanged &= ~CHANGED_CORRECTED;
  else
    mChanged |= CHANGED_CORRECTED;
  mNewCorrected = c;
}
  
// ------------------------------------------------------------------------

void KvalobsUpdate::setControlinfo(const kvalobs::kvControlInfo& ci)
{
  if (ci != mData.controlinfo())
    mChanged &= ~CHANGED_CONTROLINFO;
  else
    mChanged |= CHANGED_CONTROLINFO;
  mNewControlinfo = ci;
}
  
// ------------------------------------------------------------------------

void KvalobsUpdate::setCfailed(const std::string& cf)
{
  if (cf != mData.cfailed())
    mChanged &= ~CHANGED_CFAILED;
  else
    mChanged |= CHANGED_CFAILED;
  mNewCfailed = cf;
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

void KvalobsAccess::onUpdated(const kvData_v&)
{
  METLIBS_LOG_SCOPE();
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

  return false;
#if 0
  std::list<kvalobs::kvData> store;
  std::list<KvalobsDataPtr> modifiedObs, createdObs;
  const timeutil::ptime tbtime = boost::posix_time::microsec_clock::universal_time();
  BOOST_FOREACH(const ObsUpdate& ou, updates) {
    const SensorTime st = ou.obs->sensorTime();

    KvalobsDataPtr obs = boost::static_pointer_cast<KvalobsData>(find(st));
    bool created = false;
    if (not obs) {
      obs = boost::static_pointer_cast<KvalobsData>(create(st));
      created = true;
    }
    const bool inserted = obs->isCreated();
        
    kvalobs::kvData& d = obs->data();
    if (inserted and (ou.corrected == kvalobs::NEW_ROW or d.corrected() != kvalobs::NEW_ROW)) {
      HQC_LOG_WARN("attempt to insert unmodified new row: " << obs->sensorTime());
      continue;
    }
    if (inserted and not d.cfailed().empty()) {
      HQC_LOG_WARN("inserting new row with non-empty cfailed: " << obs->sensorTime());
    }

    if (not Helpers::float_eq()(d.corrected(), ou.corrected))
      d.corrected(ou.corrected);
    if (d.controlinfo() != ou.controlinfo)
      d.controlinfo(ou.controlinfo);
    Helpers::updateUseInfo(d);

    if (inserted) {
      Helpers::updateCfailed(d, "hqc-i");
      // specify tbtime
      d = kvalobs::kvData(d.stationID(), d.obstime(), d.original(),
          d.paramID(), timeutil::to_miTime(tbtime), d.typeID(), d.sensor(), d.level(),
          d.corrected(), d.controlinfo(), d.useinfo(), d.cfailed());
    } else {
      Helpers::updateCfailed(d, "hqc-m");
    }
    store.push_back(d);
    (created ? createdObs : modifiedObs).push_back(obs);
    METLIBS_LOG_DEBUG(LOGVAL(st) << LOGVAL(d) << LOGVAL(d.tbtime()) << LOGVAL(d.cfailed())
        << " ins=" << (inserted ? "y" : "n")
        << " create=" << (created ? "y" : "n"));
    //    << " sub=" << (isSubscribed(Helpers::sensorTimeFromKvData(d)) ? "y" : "n"));
  }
  METLIBS_LOG_DEBUG(store.size() << " to store");

  CKvalObs::CDataSource::Result_var res = mDataReinserter->insert(store);
  if (res->res == CKvalObs::CDataSource::OK) {
    BOOST_FOREACH(KvalobsDataPtr obs, modifiedObs)
        obsDataChanged(MODIFIED, obs);
    BOOST_FOREACH(KvalobsDataPtr obs, createdObs)
        obsDataChanged(CREATED, obs);
    return true;
  } else {
    std::ostringstream msg;
    msg << "saving these failed: modified:";
    BOOST_FOREACH(KvalobsDataPtr obs, modifiedObs)
        msg << ' ' << obs->sensorTime();
    msg << "; created:";
    BOOST_FOREACH(KvalobsDataPtr obs, createdObs)
        msg << ' ' << obs->sensorTime();
    HQC_LOG_WARN(msg.str());
    return false;
  }
#endif
}
