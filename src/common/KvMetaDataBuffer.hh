
#ifndef KVMETADATABUFFER_HH
#define KVMETADATABUFFER_HH 1

#include "CachedParamLimits.hh"
#include "Sensor.hh"

#include <kvalobs/kvObsPgm.h>
#include <kvalobs/kvParam.h>
#include <kvalobs/kvStation.h>
#include <kvalobs/kvTypes.h>

#include <QtCore/QObject> 

#include <list>
#include <map>
#include <set>

class ObsPgmQueryTask;
class ParamQueryTask;
class QueryTaskHelper;
class SignalTask;
class StationQueryTask;
class TypesQueryTask;

class ObsPgmRequest : public QObject
{ Q_OBJECT;
public:
  typedef std::vector<kvalobs::kvObsPgm> kvObsPgm_v;
  typedef std::map<int, kvObsPgm_v> kvObsPgm_m;
  typedef std::set<int> int_s;

  ObsPgmRequest(const int_s& stationIds);
  ~ObsPgmRequest();

  void post();

  const kvObsPgm_v& operator[](int stationId) const
    { return get(stationId); }

  const kvObsPgm_v& get(int stationId) const;

Q_SIGNALS:
  void complete();

private Q_SLOTS:
  void onTaskDone(SignalTask*);

private:
  void put(const kvObsPgm_v& op);

private:
  kvObsPgm_m mObsPgms;
  static const kvObsPgm_v sEmpty;

  QueryTaskHelper *mTaskHelper;
};

// ########################################################################

class KvMetaDataBuffer : public QObject
{ Q_OBJECT;
public:
  KvMetaDataBuffer();
  ~KvMetaDataBuffer();

  typedef std::vector<kvalobs::kvStation> kvStation_v;
  bool isKnownStation(int id);
  const kvalobs::kvStation& findStation(int id);
  const kvalobs::kvStation& findStation(const Sensor& sensor)
    { return findStation(sensor.stationId); }
  const kvStation_v& allStations();

  typedef std::vector<kvalobs::kvParam> kvParam_v;
  bool isKnownParam(int id);
  const kvalobs::kvParam& findParam(int id);
  const kvalobs::kvParam& findParam(const Sensor& sensor)
    { return findParam(sensor.paramId); }
  const kvParam_v& allParams();

  std::string findParamName(int paramId);
  std::string findParamName(const Sensor& sensor)
    { return findParamName(sensor.paramId); }
  bool isCodeParam(int paramId);
  bool isDirectionInDegreesParam(int paramId);
  bool isModelParam(int paramId);

  CachedParamLimits::ParamLimit checkPhysicalLimits(const SensorTime& st, float value);

  typedef std::vector<kvalobs::kvTypes> kvTypes_v;
  bool isKnownType(int id);
  const kvalobs::kvTypes& findType(int id);
  const kvalobs::kvTypes& findType(const Sensor& sensor)
    { return findType(sensor.typeId); }
  const kvTypes_v& allTypes();

  typedef std::vector<kvalobs::kvObsPgm> kvObsPgm_v;
  const kvObsPgm_v& findObsPgm(int stationId);
  const kvObsPgm_v& findObsPgm(const Sensor& sensor)
    { return findObsPgm(sensor.stationId); }
  void putObsPgm(const kvObsPgm_v& op);

  bool isComplete() const
    { return (mHaveStations and mHaveParams and mHaveTypes); }

  static KvMetaDataBuffer* instance()
    { return sInstance; }

public Q_SLOTS:
  void reload();

Q_SIGNALS:
  //! Emitted when params, types, and stations have been fetched.
  void complete();

private Q_SLOTS:
  void statusStations(int);
  void statusParams(int);
  void statusTypes(int);

private:
  void fetchStations();
  void fetchParams();
  void fetchTypes();

  void dropStations();
  void dropParams();
  void dropTypes();

  void sendComplete();

private:
  bool mHaveStations;
  StationQueryTask *mTaskStations;

  bool mHaveParams;
  ParamQueryTask *mTaskParams;

  bool mHaveTypes;
  TypesQueryTask *mTaskTypes;

  std::set<int> mCodeParams;
  CachedParamLimits mCachedParamLimits;

  typedef std::map<int, kvObsPgm_v> kvObsPgm_m;
  kvObsPgm_m mObsPgms;

  static KvMetaDataBuffer* sInstance;
};

#endif // KVMETADATABUFFER_HH
