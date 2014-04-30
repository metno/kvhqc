
#include "KvalobsData.hh"

#include "common/KvHelpers.hh"

KvalobsData::KvalobsData(const kvalobs::kvData& d, bool created)
  : mSensorTime(Helpers::sensorTimeFromKvData(d))
  , mKvData(d)
  , mCreated(created)
  , mModified(false)
{
}

KvalobsData::~KvalobsData()
{
}
