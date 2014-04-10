
#include "KvalobsUpdate.hh"

#include "common/Functors.hh"
#include "common/KvHelpers.hh"

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
