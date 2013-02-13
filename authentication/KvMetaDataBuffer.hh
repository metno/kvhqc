
#ifndef KVMETADATABUFFER_HH
#define KVMETADATABUFFER_HH 1

#include <kvalobs/kvObsPgm.h>
#include <kvalobs/kvParam.h>
#include <kvalobs/kvStation.h>

#include <list>
#include <set>

class KvMetaDataBuffer {
public:
    KvMetaDataBuffer();
    ~KvMetaDataBuffer();

    bool isKnownStation(int id);
    const kvalobs::kvStation& findStation(int id);
    const std::list<kvalobs::kvStation>& allStations();

    bool isKnownParam(int id);
    const kvalobs::kvParam& findParam(int id);
    const std::list<kvalobs::kvParam>& allParams();
    std::string findParamName(int paramId);
    bool isCodeParam(int paramid);
    bool isModelParam(int paramid);

    typedef std::list<kvalobs::kvObsPgm> ObsPgmList;
    const ObsPgmList& findObsPgm(int stationid);

    void reload();

    static KvMetaDataBuffer* instance()
        { return sInstance; }

private:
    void fetchStations();
    void fetchParams();

private:
    bool mHaveStations;
    std::list<kvalobs::kvStation> mStations;

    bool mHaveParams;
    std::list<kvalobs::kvParam> mParams;
    std::set<int> mCodeParams;

    typedef std::map<int, ObsPgmList> ObsPgms_t;
    ObsPgms_t mObsPgms;

    static KvMetaDataBuffer* sInstance;
};

#endif // KVMETADATABUFFER_HH
