// -*- c++ -*-

#ifndef StationInfoBuffer_hh
#define StationInfoBuffer_hh

#include "util/timeutil.hh"
#include <list>
#include <string>
#include <vector>

struct listStat_t {
  std::string name;    // listStatName
  int stationid;       // listStatNum
  float altitude;      // listStatHoh
  std::string fylke;   // listStatFylke
  std::string kommune; // listStatKommune
  int municipid;
  int wmonr;           // listStatWeb
  int pri;             // listStatPri
  bool coast;
};
typedef std::list<listStat_t> listStat_l;

class StationInfoBuffer {
public:
  typedef std::vector<int> manual_types_t;

  StationInfoBuffer();
  ~StationInfoBuffer();

  virtual bool isConnected();

  const listStat_l& getStationDetails();

  static StationInfoBuffer* instance()
    { return sInstance; }

  const manual_types_t& getManualTypes();

protected:
  virtual void readStationInfo();
  bool writeToStationFile();
  virtual void refreshIfOld();

private:
  bool readFromStationFile();

protected:
  listStat_l listStat;
  manual_types_t mManualTypes;

private:
  static StationInfoBuffer* sInstance;
  timeutil::ptime mLastStationListUpdate;
};

#endif // StationInfoBuffer_hh
