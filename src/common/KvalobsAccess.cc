
#include "KvalobsAccess.hh"
#include "KvalobsAccessPrivate.hh"

#include "KvalobsData.hh"
#include "KvalobsUpdate.hh"
#include "sqlutil.hh"
#include "Functors.hh"
#include "KvHelpers.hh"
#include "HqcApplication.hh"
#include "QueryTask.hh"

#include <QtCore/QVariant>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

#include <boost/algorithm/string.hpp>
#include <boost/make_shared.hpp>

#define MILOGGER_CATEGORY "kvhqc.KvalobsAccess"
#include "common/ObsLogging.hh"

namespace /*anonymous*/ {

const QString QDBNAME = "kvalobs_bg";
const QString DBVERSION = "kvalobs:1";

class QtSqlRow : public ResultRow
{
public:
  QtSqlRow(QSqlQuery& query) : mQuery(query) { }

  QVariant value(int index) const
    { return mQuery.value(index); }

  int getInt(int index) const
    { return mQuery.value(index).toInt(); }

  float getFloat(int index) const
    { return mQuery.value(index).toFloat(); }
  
  QString getQString(int index) const
    { return mQuery.value(index).toString(); }

  std::string getStdString(int index) const
    { return getQString(index).toStdString(); }

private:
  QSqlQuery& mQuery;
};

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

void KvalobsHandler::queryTask(QueryTask* qtask)
{
  METLIBS_LOG_SCOPE();

  QSqlQuery query(mKvalobsDB);
  QtSqlRow row(query);

  const QString sql = qtask->querySql(DBVERSION);
  if (query.exec(sql)) {
    while (query.next())
      qtask->notifyRow(row);
    qtask->notifyDone();
  } else {
    HQC_LOG_ERROR("query '" << sql << "' failed: " << query.lastError().text());
    qtask->notifyError(query.lastError().text());
  }
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
  if (not mDataReinserter) {
    METLIBS_LOG_DEBUG("not authorized");
    return false;
  }

  METLIBS_LOG_DEBUG(updates.size() << " updates");
  if (updates.empty())
    return true;

  AbstractReinserter::kvData_l dataUpdate, dataInsert;
  ObsData_pv modifiedObs, createdObs;
  const timeutil::ptime tbtime = boost::posix_time::microsec_clock::universal_time();
  for (ObsUpdate_pv::const_iterator it = updates.begin(); it != updates.end(); ++it) {
    KvalobsUpdate_p ou = boost::static_pointer_cast<KvalobsUpdate>(*it);

    if (ou->obs()) {
      KvalobsData_p d = Helpers::modifiedData(ou->obs(), ou->corrected(), ou->controlinfo(), ou->cfailed());
      Helpers::updateCfailed(d->data(), "hqc-m");
      modifiedObs.push_back(d);
      dataUpdate.push_back(d->data());
    } else {
      KvalobsData_p d = Helpers::createdData(ou->sensorTime(), tbtime, ou->corrected(), ou->controlinfo(), ou->cfailed());
      Helpers::updateCfailed(d->data(), "hqc-i");
      createdObs.push_back(d);
      dataInsert.push_back(d->data());
    }
  }
  METLIBS_LOG_DEBUG(dataUpdate.size() << " to update, " << dataInsert.size() << " to insert");

  if (mDataReinserter->storeChanges(dataUpdate, dataInsert)) {
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
