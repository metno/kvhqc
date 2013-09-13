
#include "KvalobsModelData.hh"

#include "KvHelpers.hh"

KvalobsModelData::KvalobsModelData(const kvalobs::kvModelData& d)
  : mKvData(d)
{
}

KvalobsModelData::~KvalobsModelData()
{
}

SensorTime KvalobsModelData::sensorTime() const
{
  return Helpers::sensorTimeFromKvModelData(mKvData);
}
