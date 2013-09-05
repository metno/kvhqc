
#ifndef KVMETADATABUFFER_HH
#define KVMETADATABUFFER_HH 1

#include "Sensor.hh"

#include <kvalobs/kvObsPgm.h>
#include <kvalobs/kvParam.h>
#include <kvalobs/kvStation.h>
#include <kvalobs/kvTypes.h>

#include <list>
#include <set>

class KvMetaDataBuffer {
public:
  KvMetaDataBuffer();
  ~KvMetaDataBuffer();

  bool isKnownStation(int id);
  const kvalobs::kvStation& findStation(int id);
  const kvalobs::kvStation& findStation(const Sensor& sensor)
    { return findStation(sensor.stationId); }
  const std::list<kvalobs::kvStation>& allStations();

  bool isKnownParam(int id);
  const kvalobs::kvParam& findParam(int id);
  const kvalobs::kvParam& findParam(const Sensor& sensor)
    { return findParam(sensor.paramId); }
  const std::list<kvalobs::kvParam>& allParams();
  std::string findParamName(int paramId);
  std::string findParamName(const Sensor& sensor)
    { return findParamName(sensor.paramId); }
  bool isCodeParam(int paramId);
  bool isModelParam(int paramId);
  bool checkPhysicalLimits(int paramid, float value);

  bool isKnownType(int id);
  const kvalobs::kvTypes& findType(int id);
  const kvalobs::kvTypes& findType(const Sensor& sensor)
    { return findType(sensor.typeId); }
  const std::list<kvalobs::kvTypes>& allTypes();

  typedef std::list<kvalobs::kvObsPgm> ObsPgmList;
  const ObsPgmList& findObsPgm(int stationId);
  const ObsPgmList& findObsPgm(const Sensor& sensor)
    { return findObsPgm(sensor.stationId); }
  void findObsPgm(const std::set<long>& stationids);

  void reload();

  static KvMetaDataBuffer* instance()
    { return sInstance; }

private:
  void fetchStations();
  void fetchParams();
  void fetchTypes();

private:
  bool mHaveStations;
  std::list<kvalobs::kvStation> mStations;

  bool mHaveParams;
  std::list<kvalobs::kvParam> mParams;
  std::set<int> mCodeParams;
  typedef std::map<int, std::pair<float, float> > ParamLimits_t;
  ParamLimits_t mParamLimits;

  bool mHaveTypes;
  std::list<kvalobs::kvTypes> mTypes;

  typedef std::map<int, ObsPgmList> ObsPgms_t;
  ObsPgms_t mObsPgms;

  static KvMetaDataBuffer* sInstance;
};

#endif // KVMETADATABUFFER_HH
