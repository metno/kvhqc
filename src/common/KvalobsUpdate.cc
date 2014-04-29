
#include "KvalobsUpdate.hh"

#include "KvHelpers.hh"

KvalobsUpdate::KvalobsUpdate(const SensorTime& st)
  : mSensorTime(st)
{
  const kvalobs::kvData missing = Helpers::getMissingKvData(st);
  mCorrected = missing.corrected();
  mControlinfo = missing.controlinfo();
  mCfailed = missing.cfailed();
}

KvalobsUpdate::KvalobsUpdate(ObsData_p obs)
  : mSensorTime(obs->sensorTime())
  , mObs(obs)
  , mCorrected(obs->corrected())
  , mControlinfo(obs->controlinfo())
  , mCfailed(obs->cfailed())
{
}
