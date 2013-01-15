
#include "KvStationBuffer.hh"

#include <kvcpp/KvApp.h>

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

namespace /* anonymous */ {
struct find_stationid : public std::unary_function<bool, kvalobs::kvStation>
{
    int stationid;
    find_stationid(int s) : stationid(s) { }
    bool operator()(const kvalobs::kvStation& s) const
        { return s.stationID() == stationid; }
};
} // anonymous namespace

const kvalobs::kvStation& KvStationBuffer::findStation(int id)
{
    if (mStations.empty())
        fetch();

    std::list<kvalobs::kvStation>::const_iterator it
        = std::find_if(mStations.begin(), mStations.end(), find_stationid(id));
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

void KvStationBuffer::fetch() {
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
