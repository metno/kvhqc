
#include "CachedParamLimits.hh"

#include "HqcApplication.hh"
#include "HqcSystemDB.hh"
#include "KvHelpers.hh"
#include "StationParamSQLTask.hh"
#include "SyncTask.hh"

#include <kvcpp/KvApp.h>
#include <kvalobs/kvStationParam.h>

#include <QVariant>
#include <QSqlQuery>

#ifdef DEBUG_ME_ON
#define MILOGGER_CATEGORY "kvhqc.CachedParamLimits"
#include "common/ObsLogging.hh"
#define DEBUG_ME(x) x
#else
#define DEBUG_ME(x) do { } while(false)
#endif

CachedParamLimits::CachedParamLimits()
    : sensor_limits(64)
{
}

CachedParamLimits::~CachedParamLimits()
{
}

void CachedParamLimits::Limits::reset()
{
  have_max = have_high = have_low = have_min = false;
  fromtime = totime = timeutil::ptime();
}

CachedParamLimits::ParamLimit CachedParamLimits::check(const SensorTime& st, float value)
{
  DEBUG_ME(METLIBS_LOG_SCOPE(LOGVAL(st) << LOGVAL(value)));
  if (value == kvalobs::MISSING or value == kvalobs::REJECTED)
    return Ok;

  const Sensor& sensor = st.sensor;

  bool fetch = !sensor_limits.has(sensor);
  if (!fetch) {
    const Limits& limits = sensor_limits.get(sensor);
    if (limits.fromtime.is_not_a_date_time() || st.time < limits.fromtime || (!limits.totime.is_not_a_date_time() && st.time >= limits.totime)) {
      fetch = true;
    }
  }
  if (fetch) {
    Limits limits;

    const QString metadata = fetchMetaData(st, limits.fromtime, limits.totime);
    if (not metadata.isNull()) {
      parseMetadata(metadata, limits);
    }

    if (!(limits.have_max && limits.have_min))
      fetchLimitsFromSystemDB(st, limits);

    sensor_limits.put(sensor, limits);
  }

  const Limits& limits = sensor_limits.get(sensor);
  if ((limits.have_max && value > limits.param_max) || (limits.have_min && value < limits.param_min))
    return OutsideMinMax;

  if ((limits.have_high && value > limits.param_high) || (limits.have_low && value < limits.param_low))
    return OutsideHighLow;

  return Ok;
}

QString CachedParamLimits::fetchMetaData(const SensorTime& st, timeutil::ptime& fromtime, timeutil::ptime& totime)
{
  DEBUG_ME(METLIBS_LOG_SCOPE(LOGVAL(st)));

  QString metadata;
  StationParamSQLTask task(st, QueryTask::PRIORITY_SYNC);

  syncTask(&task, mHandler);

  fromtime = totime = timeutil::ptime();

  for (size_t i=0; i<task.metadata().size(); ++i) {
    const std::string& md = task.metadata().at(i);
    if (md.find("high") == std::string::npos)
      continue;
    const timeutil::ptime& ft = task.fromtimes().at(i);
    if (ft <= st.time and (fromtime.is_not_a_date_time() or fromtime < ft)) {
      metadata = QString::fromStdString(task.metadata().at(i));
      fromtime = ft;
    }
    if (ft > st.time and (totime.is_not_a_date_time() or totime > ft)) {
      totime = ft;
    }
  }

  DEBUG_ME(METLIBS_LOG_DEBUG(LOGVAL(metadata)));
  return metadata;
}

void CachedParamLimits::parseMetadata(const QString& metadata, Limits& limits)
{
  limits.reset();

  const QStringList lines = metadata.split(QChar('\n'));
  if (lines.length() != 2)
    return;

  const QStringList keys = lines.at(0).split(QChar(';'));
  const QStringList values = lines.at(1).split(QChar(';'));
  if (keys.length() != values.length())
    return;

  for (int i=0; i<keys.length(); ++i) {
    if (keys.at(i) == "max") {
      limits.param_max = values.at(i).toFloat();
      limits.have_max = true;
    } else if (keys.at(i) == "min") {
      limits.param_min = values.at(i).toFloat();
      limits.have_min = true;
    } else if (keys.at(i) == "high") {
      limits.param_high = values.at(i).toFloat();
      limits.have_high = true;
    } else if (keys.at(i) == "low") {
      limits.param_low = values.at(i).toFloat();
      limits.have_low = true;
    }
  }
}

void CachedParamLimits::fetchLimitsFromSystemDB(const SensorTime& st, Limits& limits)
{
  if (HqcSystemDB::paramLimits(st.sensor.paramId, limits.param_min, limits.param_max)) {
    limits.fromtime = timeutil::from_iso_extended_string("1800-01-01 00:00:00");
    limits.have_max = limits.have_min = true;
  }
}
