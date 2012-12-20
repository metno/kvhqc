
#include "KvalobsModelData.hh"

#include "Helpers.hh"

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
