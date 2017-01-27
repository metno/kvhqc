
#include "KvServiceHelper.hh"

#include <kvalobs/kvOperator.h>
#include <kvalobs/kvStationParam.h>

#define MILOGGER_CATEGORY "kvhqc.KvServiceHelper"
#include "util/HqcLogging.hh"

KvServiceHelper* KvServiceHelper::sInstance = 0;

KvServiceHelper::KvServiceHelper(std::shared_ptr<kvservice::KvApp> app)
  : mApp(app)
  , mKvalobsAvailable((bool)mApp)
{
  sInstance = this;
}

KvServiceHelper::~KvServiceHelper()
{
  sInstance = 0;
}

bool KvServiceHelper::checkKvalobsAvailability()
{
  if (!mApp)
    return false;
  std::list<kvalobs::kvStationParam> stParam;
  return updateKvalobsAvailability(mApp->getKvStationParam(stParam, 345345, 345345, 0));
}

bool KvServiceHelper::updateKvalobsAvailability(bool available)
{
  if (available != mKvalobsAvailable) {
    mKvalobsAvailable = available;
    Q_EMIT kvalobsAvailable(mKvalobsAvailable);
  }
  return available;
}

int KvServiceHelper::identifyOperator(const QString& username)
{
  if (!mApp) {
    updateKvalobsAvailability(false);
    return -1;
  }

  typedef std::list<kvalobs::kvOperator> kvOperator_l;
  kvOperator_l operators;
  if (!mApp->getKvOperator(operators)) {
    updateKvalobsAvailability(false);
    return -1;
  }
  updateKvalobsAvailability(true);

  for (kvOperator_l::const_iterator it = operators.begin(); it != operators.end(); ++it) {
    if (username == QString::fromStdString(it->username()))
      return it->userID();
  }
  return -1;
}
