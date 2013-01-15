
#ifndef KVSTATIONBUFFER_HH
#define KVSTATIONBUFFER_HH 1

#include <kvalobs/kvStation.h>

class KvStationBuffer {
public:
    KvStationBuffer();
    ~KvStationBuffer();

    const kvalobs::kvStation& findStation(int id);

    const std::list<kvalobs::kvStation>& allStations();

    void reload();

    static KvStationBuffer* instance()
        { return sInstance; }

private:
    void fetch();

private:
    std::list<kvalobs::kvStation> mStations;

    static KvStationBuffer* sInstance;
};

#endif // KVSTATIONBUFFER_HH
