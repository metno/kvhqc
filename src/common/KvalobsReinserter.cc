
#include "KvalobsReinserter.hh"
#include "common/identifyUser.h"
#include <QtCore/QString>

bool KvalobsReinserter::authenticate()
{
  QString userName = "?";
  mDataReinserter.reset(Authentication::identifyUser(0, kvservice::KvApp::kvApp, "ldap-oslo.met.no", userName));
  return mDataReinserter.get() != 0;
}

bool KvalobsReinserter::storeChanges(const kvData_l& toUpdate, const kvData_l& toInsert)
{
  if (not authenticated())
    authenticate();

  if (not authenticated())
    return false;

  kvData_l merged(toUpdate.begin(), toUpdate.end());
  merged.insert(merged.end(), toInsert.begin(), toInsert.end());

  const HqcDataReinserter::Result res = mDataReinserter->insert(merged);
  return (res->res == CKvalObs::CDataSource::OK);
}
