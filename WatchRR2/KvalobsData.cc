
#include "KvalobsData.hh"

#include "Helpers.hh"

KvalobsData::KvalobsData(const kvalobs::kvData& d)
    : mKvData(d)
{
}

KvalobsData::~KvalobsData()
{
}

SensorTime KvalobsData::sensorTime() const
{
    return Helpers::sensorTimeFromKvData(mKvData);
}
