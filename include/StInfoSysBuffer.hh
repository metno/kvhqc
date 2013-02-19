// -*- c++ -*-

#ifndef StInfoSysBuffer_hh
#define StInfoSysBuffer_hh

#include "timeutil.hh"
#include <string>

namespace miutil {
namespace conf {
class ConfSection;
} // namespace conf
} // namespace miutil

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

class StInfoSysBuffer {
public:
    StInfoSysBuffer(miutil::conf::ConfSection* conf);
    ~StInfoSysBuffer();

    bool isConnected();
    const listStat_l& getStationDetails();

    static StInfoSysBuffer* instance()
        { return sInstance; }

private:
    bool readFromStInfoSys();
    bool readFromStationFile();

private:
    static StInfoSysBuffer* sInstance;
    listStat_l listStat;
    timeutil::ptime mLastStationListUpdate;
};

#endif // StInfoSysBuffer_hh
