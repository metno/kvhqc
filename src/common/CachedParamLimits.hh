#ifndef CACHEDPARAMLIMITS_HH
#define CACHEDPARAMLIMITS_HH 1

#include "QueryTaskHandler.hh"
#include "Sensor.hh"

#include <QString>

class CachedParamLimits {
public:
  enum ParamLimit { Ok, OutsideHighLow, OutsideMinMax };

  CachedParamLimits();
  ~CachedParamLimits();

  ParamLimit check(const SensorTime& st, float value);

  void setHandler(QueryTaskHandler_p handler)
      { mHandler = handler; }

private:
  void reset();
  QString fetchMetaData(const SensorTime& st);
  void parseMetadata(const QString& metadata);
  void fetchLimitsFromSystemDB(const SensorTime& st);

private:
  boost::shared_ptr<QueryTaskHandler> mHandler;

  Sensor sensor;
  timeutil::ptime fromtime, totime;
  float param_max, param_high, param_low, param_min;
  bool have_max, have_high, have_low, have_min;
};

#endif // CACHEDPARAMLIMITS_HH
