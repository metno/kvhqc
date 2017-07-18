
#include "KvalobsReinserter.hh"
#include "common/identifyUser.h"

#include <kvalobs/kvDataOperations.h>

#include <QString>

#define MILOGGER_CATEGORY "kvhqc.KvalobsReinserter"
#include "util/HqcLogging.hh"

namespace internal_
{

void updateUseAddCFailed(kvalobs::kvData &d)
{
    kvalobs::kvControlInfo cinfo = d.controlinfo();
    if (cinfo.flag(kvalobs::flag::fhqc) == 0) {
      cinfo.set(kvalobs::flag::fhqc, 3);
      d.controlinfo(cinfo);
      HQC_LOG_ERROR("inserting data with fhqc==0, forced to fhqc==3: " << d);
    }

    kvalobs::kvUseInfo ui = d.useinfo();
    ui.setUseFlags(cinfo);
    ui.addToErrorCount();
    d.useinfo(ui);
}

} // namespace internal_

KvalobsReinserter::KvalobsReinserter(std::shared_ptr<kvservice::KvApp> app)
  : mApp(app)
{
}

bool KvalobsReinserter::authenticate()
{
  QString userName = "?";
  const int userid = Authentication::identifyUser(0, "ldap-oslo.met.no", userName);
  if (userid >= 0) {
    mDataReinserter.reset(new Reinserter_t(mApp.get(), userid));
  } else {
    mDataReinserter.reset(0);
  }
  return (bool)mDataReinserter;
}

bool KvalobsReinserter::storeChanges(const kvData_l& toUpdate, const kvData_l& toInsert)
{
  if (not authenticated())
    authenticate();

  if (not authenticated())
    return false;

  kvData_l merged(toUpdate.begin(), toUpdate.end());
  merged.insert(merged.end(), toInsert.begin(), toInsert.end());

  const CKvalObs::CDataSource::Result_var res = mDataReinserter->insert(merged);
  if (res->res == CKvalObs::CDataSource::OK) {
    return true;
  } else {
    HQC_LOG_WARN("could not store data, message='" << res->message << "'");
    return false;
  }
}
