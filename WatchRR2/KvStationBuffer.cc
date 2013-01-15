
#include "KvStationBuffer.hh"
#include "BusyIndicator.h"
#include "Helpers.hh"

#include <kvcpp/KvApp.h>

#define NDEBUG
#include "w2debug.hh"

KvStationBuffer* KvStationBuffer::sInstance = 0;

KvStationBuffer::KvStationBuffer()
{
    assert(not sInstance);
    sInstance = this;
}

KvStationBuffer::~KvStationBuffer()
{
    sInstance = 0;
}

const kvalobs::kvStation& KvStationBuffer::findStation(int id)
{
    if (mStations.empty())
        fetch();

    std::list<kvalobs::kvStation>::const_iterator it
        = std::find_if(mStations.begin(), mStations.end(), Helpers::station_by_id(id));
    if (it == mStations.end())
        throw "station not found";
    return *it;
}

const std::list<kvalobs::kvStation>& KvStationBuffer::allStations()
{
    if (mStations.empty())
        fetch();
    return mStations;
}

void KvStationBuffer::fetch()
{
    LOG_SCOPE();
    BusyIndicator wait;
    mStations.clear();
    if (not kvservice::KvApp::kvApp->getKvStations(mStations)) {
        std::cerr << "could not fetch station list" << std::endl;
    }
}

void KvStationBuffer::reload()
{
    // FIXME dangerous if a reference is kept somewhere
    mStations.clear();
}
