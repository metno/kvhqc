
#ifndef KVMETADATABUFFER_HH
#define KVMETADATABUFFER_HH 1

#include "CachedParamLimits.hh"
#include "Sensor.hh"
#include "KvTypedefs.hh"
#include "QueryTaskHandler.hh"

#include <QtCore/QObject> 

#include <map>

class QueryTaskHelper;

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

  void setHandler(QueryTaskHandler_p handler)
      { mHandler = handler; }

  static KvMetaDataBuffer* instance()
    { return sInstance; }

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

  QueryTaskHandler_x handler();

private:
  bool mHaveStations;
  QueryTaskHelper *mTaskStations;

  bool mHaveParams;
  QueryTaskHelper *mTaskParams;

  bool mHaveTypes;
  QueryTaskHelper *mTaskTypes;

  hqc::int_s mCodeParams;
  CachedParamLimits mCachedParamLimits;

  typedef std::map<int, hqc::kvObsPgm_v> kvObsPgm_m;
  kvObsPgm_m mObsPgms;

  boost::shared_ptr<QueryTaskHandler> mHandler;

  static KvMetaDataBuffer* sInstance;
};

#endif // KVMETADATABUFFER_HH
