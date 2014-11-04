
#include "CachedParamLimits.hh"

#include "HqcApplication.hh"
#include "HqcSystemDB.hh"
#include "KvHelpers.hh"
#include "StationParamSQLTask.hh"
#include "SyncTask.hh"

#include <kvcpp/KvApp.h>
#include <kvalobs/kvStationParam.h>

#include <QtCore/QVariant>
#include <QtSql/QSqlQuery>

#ifdef DEBUG_ME_ON
#define MILOGGER_CATEGORY "kvhqc.CachedParamLimits"
#include "common/ObsLogging.hh"
#define DEBUG_ME(x) x
#else
#define DEBUG_ME(x) do { } while(false)
#endif

CachedParamLimits::CachedParamLimits()
{
  reset();
}

CachedParamLimits::~CachedParamLimits()
{
}

void CachedParamLimits::reset()
{
  have_max = have_high = have_low = have_min = false;
}

CachedParamLimits::ParamLimit CachedParamLimits::check(const SensorTime& st, float value)
{
  DEBUG_ME(METLIBS_LOG_SCOPE(LOGVAL(st) << LOGVAL(value)));
  if (value == kvalobs::MISSING or value == kvalobs::REJECTED)
    return Ok;

  if (not (sensor.valid() and eq_Sensor()(st.sensor, sensor))
      or fromtime.is_not_a_date_time() or st.time < fromtime
      or (not totime.is_not_a_date_time() and st.time >= totime))
  {
    DEBUG_ME(METLIBS_LOG_DEBUG(LOGVAL(sensor) << LOGVAL(fromtime) << LOGVAL(totime)));
    sensor = st.sensor;

    const QString metadata = fetchMetaData(st);
    if (not metadata.isNull())
      parseMetadata(metadata);

    if (not (have_max and have_min))
      fetchLimitsFromSystemDB(st);

    DEBUG_ME(METLIBS_LOG_DEBUG(LOGVAL(fromtime) << LOGVAL(totime)
            << LOGVAL(have_min) << LOGVAL(have_max) << LOGVAL(param_min) << LOGVAL(param_max)));
  }

  if ((have_max and value > param_max) or (have_min and value < param_min))
    return OutsideMinMax;
  
  if ((have_high and value > param_high) or (have_low and value < param_low))
    return OutsideHighLow;

  return Ok;
}

QString CachedParamLimits::fetchMetaData(const SensorTime& st)
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

void CachedParamLimits::parseMetadata(const QString& metadata)
{
  have_max = have_min = have_high = have_low = false;

  const QStringList lines = metadata.split(QChar('\n'));
  if (lines.length() != 2)
    return;

  const QStringList keys = lines.at(0).split(QChar(';'));
  const QStringList values = lines.at(1).split(QChar(';'));
  if (keys.length() != values.length())
    return;

  for (int i=0; i<keys.length(); ++i) {
    if (keys.at(i) == "max") {
      param_max = values.at(i).toFloat();
      have_max = true;
    } else if (keys.at(i) == "min") {
      param_min = values.at(i).toFloat();
      have_min = true;
    } else if (keys.at(i) == "high") {
      param_high = values.at(i).toFloat();
      have_high = true;
    } else if (keys.at(i) == "low") {
      param_low = values.at(i).toFloat();
      have_low = true;
    }
  }
}

void CachedParamLimits::fetchLimitsFromSystemDB(const SensorTime& st)
{
  fromtime = totime = timeutil::ptime();
  if (HqcSystemDB::paramLimits(st.sensor.paramId, param_min, param_max)) {
    fromtime = timeutil::from_iso_extended_string("1800-01-01 00:00:00");
    have_max  = have_min = true;
    have_high = have_low = false;
  }
}
