// -*- c++ -*-

#ifndef StationInfoBuffer_hh
#define StationInfoBuffer_hh

#include "timeutil.hh"
#include <string>

struct listStat_t {
    std::string name;    // listStatName
    int stationid;       // listStatNum
    float altitude;      // listStatHoh
    int environment;     // listStatType
    std::string fylke;   // listStatFylke
    std::string kommune; // listStatKommune
    std::string wmonr;   // listStatWeb
    std::string pri;     // listStatPri
    timeutil::ptime fromtime;
    timeutil::ptime totime;
    bool coast;
};
typedef std::list<listStat_t> listStat_l;

class StationInfoBuffer {
public:
    StationInfoBuffer();
    ~StationInfoBuffer();

    virtual bool isConnected();

    const listStat_l& getStationDetails();

    static StationInfoBuffer* instance()
        { return sInstance; }

protected:
    virtual void readStationInfo();
    bool writeToStationFile();

private:
    bool readFromStationFile();

    std::string localCacheFileName() const;

protected:
    listStat_l listStat;

private:
    static StationInfoBuffer* sInstance;
    timeutil::ptime mLastStationListUpdate;
};

#endif // StationInfoBuffer_hh
