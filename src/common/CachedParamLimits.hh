#ifndef CACHEDPARAMLIMITS_HH
#define CACHEDPARAMLIMITS_HH 1

#include "QueryTaskHandler.hh"
#include "Sensor.hh"
#include "SensorHash.hh"
#include "util/lru_cache.hh"

#include <QString>

class CachedParamLimits {
private:
  struct Limits {
    timeutil::ptime fromtime, totime;
    float param_max, param_high, param_low, param_min;
    bool have_max, have_high, have_low, have_min;

    Limits() { reset(); }
    void reset();
  };
  typedef lru_cache<Sensor, Limits, hash_Sensor, eq_Sensor> sensor_limits_t;

public:
  enum ParamLimit { Ok, OutsideHighLow, OutsideMinMax };

  CachedParamLimits();
  ~CachedParamLimits();

  ParamLimit check(const SensorTime& st, float value);

  void setHandler(QueryTaskHandler_p handler)
      { mHandler = handler; }

private:
  QString fetchMetaData(const SensorTime& st, timeutil::ptime & fromtime, timeutil::ptime & totime);
  void parseMetadata(const QString& metadata, Limits& limits);
  void fetchLimitsFromSystemDB(const SensorTime& st, Limits& limits);

private:
  std::shared_ptr<QueryTaskHandler> mHandler;
  sensor_limits_t sensor_limits;
};

#endif // CACHEDPARAMLIMITS_HH
