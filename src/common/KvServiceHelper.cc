
#include "KvServiceHelper.hh"

#include <kvalobs/kvOperator.h>
#include <kvalobs/kvStationParam.h>
#include <kvcpp/KvApp.h>

#define MILOGGER_CATEGORY "kvhqc.KvServiceHelper"
#include "util/HqcLogging.hh"

KvServiceHelper* KvServiceHelper::sInstance = 0;

KvServiceHelper::KvServiceHelper()
    : mKvalobsAvailable(true)
{
  sInstance = this;
}

KvServiceHelper::~KvServiceHelper()
{
  sInstance = 0;
}

bool KvServiceHelper::checkKvalobsAvailability()
{
  std::list<kvalobs::kvStationParam> stParam;
  return updateKvalobsAvailability(kvservice::KvApp::kvApp->getKvStationParam(stParam, 345345, 345345, 0));
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
  if (not kvservice::KvApp::kvApp) {
    updateKvalobsAvailability(false);
    return -1;
  }

  typedef std::list<kvalobs::kvOperator> kvOperator_l;
  kvOperator_l operators;
  if (not kvservice::KvApp::kvApp->getKvOperator(operators)) {
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
