
#ifndef KVMETADATABUFFER_HH
#define KVMETADATABUFFER_HH 1

#include "CachedParamLimits.hh"
#include "Sensor.hh"
#include "TimeSpan.hh"
#include "KvTypedefs.hh"
#include "QueryTaskHandler.hh"

#include <QObject> 

#include <map>

class ObsPgmRequest;
class QueryTaskHelper;

class KvMetaDataBuffer : public QObject
{ Q_OBJECT;
public:
  KvMetaDataBuffer();
  ~KvMetaDataBuffer();

  bool isKnownStation(int id);
  const kvalobs::kvStation& findStation(int id);
  const kvalobs::kvStation& findStation(const Sensor& sensor)
    { return findStation(sensor.stationId); }
  const hqc::kvStation_v& allStations();

  bool isKnownParam(int id);
  const kvalobs::kvParam& findParam(int id);
  const kvalobs::kvParam& findParam(const Sensor& sensor)
    { return findParam(sensor.paramId); }
  const hqc::kvParam_v& allParams();

  std::string findParamName(int paramId);
  std::string findParamName(const Sensor& sensor)
    { return findParamName(sensor.paramId); }
  bool isCodeParam(int paramId);
  bool isDirectionInDegreesParam(int paramId);
  bool isModelParam(int paramId);

  CachedParamLimits::ParamLimit checkPhysicalLimits(const SensorTime& st, float value);

  bool isKnownType(int id);
  const kvalobs::kvTypes& findType(int id);
  const kvalobs::kvTypes& findType(const Sensor& sensor)
    { return findType(sensor.typeId); }
  const hqc::kvTypes_v& allTypes();

  const hqc::kvObsPgm_v& findObsPgm(int stationId);
  const hqc::kvObsPgm_v& findObsPgm(const Sensor& sensor)
    { return findObsPgm(sensor.stationId); }
  void putObsPgm(const hqc::kvObsPgm_v& op);


  bool isComplete() const
    { return (mHaveStations and mHaveParams and mHaveTypes); }

  void setHandler(QueryTaskHandler_p handler);

  QueryTaskHandler_p handler();

  static KvMetaDataBuffer* instance()
    { return sInstance; }

  // ====================
  // utility functions

  QString stationInfo(int stationId);
  QString paramInfo(int paramId);
  QString typeInfo(int typeId);
  QString paramName(int paramId);

  int nearestStationId(float lon, float lat, float maxDistanceKm = 10);
  hqc::kvStation_v findNeighborStations(int stationId, float maxDistanceKm = 100);
  hqc::int_s findNeighborStationIds(int stationId, float maxDistanceKm = 100);
  void addNeighbors(Sensor_v& neighbors, const Sensor& sensor, const TimeSpan& time,
      const ObsPgmRequest* obsPgms, int maxNeighbors);
  Sensor_v relatedSensors(const Sensor& s, const TimeSpan& time, const std::string& viewType,
      const ObsPgmRequest* obsPgms);

  // end utility functions
  // ====================

public Q_SLOTS:
  void reload();

Q_SIGNALS:
  //! Emitted when params, types, and stations have been fetched.
  void complete();

private Q_SLOTS:
  void taskDoneStations();
  void taskDoneParams();
  void taskDoneTypes();

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
  QueryTaskHelper *mTaskStations;

  bool mHaveParams;
  QueryTaskHelper *mTaskParams;

  bool mHaveTypes;
  QueryTaskHelper *mTaskTypes;

  hqc::int_s mCodeParams;
  CachedParamLimits mCachedParamLimits;
  hqc::int_s mDirectionParams;

  typedef std::map<int, hqc::kvObsPgm_v> kvObsPgm_m;
  kvObsPgm_m mObsPgms;

  std::shared_ptr<QueryTaskHandler> mHandler;

  static KvMetaDataBuffer* sInstance;
};

#endif // KVMETADATABUFFER_HH
