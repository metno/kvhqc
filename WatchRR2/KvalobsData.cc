
#include "KvalobsData.hh"

#include "Helpers.hh"

KvalobsData::KvalobsData(const kvalobs::kvData& d, bool created)
    : mKvData(d)
    , mCreated(created)
{
}

KvalobsData::~KvalobsData()
{
}

SensorTime KvalobsData::sensorTime() const
{
    return Helpers::sensorTimeFromKvData(mKvData);
}
