
#include "CachedParamLimits.hh"

#include "common/HqcApplication.hh"
#include "common/KvHelpers.hh"
#include "util/stringutil.hh"

#include <kvcpp/KvApp.h>
#include <kvalobs/kvStationParam.h>

#include <QtCore/QVariant>
#include <QtSql/QSqlQuery>

#define MILOGGER_CATEGORY "kvhqc.CachedParamLimits"
#include "common/ObsLogging.hh"

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
  fromtime = totime = timeutil::ptime();
}

CachedParamLimits::ParamLimit CachedParamLimits::check(const SensorTime& st, float value)
{
  if (value == kvalobs::MISSING or value == kvalobs::REJECTED)
    return Ok;

  if (not (sensor.valid() and eq_Sensor()(st.sensor, sensor))
      or fromtime.is_not_a_date_time() or st.time < fromtime
      or (not totime.is_not_a_date_time() and st.time >= totime))
  {
    reset();
    sensor = st.sensor;

#if 0 // FIXME
    const QString metadata = fetchMetaData(st);
    if (not metadata.isNull())
      parseMetadata(metadata);
#endif

    if (not (have_max and have_min))
      fetchLimitsFromSystemDB(st);
  }

  if ((have_max and value > param_max)
      or (have_min and value < param_min))
    return OutsideMinMax;

  if ((have_high and value > param_high)
      or (have_low and value < param_low))
    return OutsideHighLow;

  return Ok;
}

QString CachedParamLimits::fetchMetaData(const SensorTime& st)
{
  METLIBS_LOG_SCOPE(LOGVAL(st));
  if (not kvservice::KvApp::kvApp)
    return QString();

  QString metadata;
  const int day  = st.time.date().day_of_year();
  std::list<kvalobs::kvStationParam> stParam;
  if (kvservice::KvApp::kvApp->getKvStationParam(stParam, st.sensor.stationId, st.sensor.paramId, day)) {
    for (std::list<kvalobs::kvStationParam>::const_iterator it = stParam.begin(); it != stParam.end(); ++it) {
      if (it->hour() != -1) {
        HQC_LOG_ERROR("station_param.hour != -1 for " << st.sensor << ", ignored");
        continue;
      }
      if (it->sensor() == st.sensor.sensor and it->level() == st.sensor.level) {
        if (it->fromtime() <= st.time
            and (fromtime.is_not_a_date_time()
                or fromtime < it->fromtime()))
        {
          metadata = Helpers::fromUtf8(it->metadata());
          fromtime = it->fromtime();
        }
        if (it->fromtime() > st.time
            and (totime.is_not_a_date_time() or totime > it->fromtime()))
        {
          totime = it->fromtime();
        }
      }
    }
  }
  METLIBS_LOG_DEBUG(LOGVAL(metadata));
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
  if (not hqcApp)
    return;

  QSqlQuery query(hqcApp->systemDB());
  query.exec("SELECT low, high FROM slimits WHERE paramid = ?");
  query.bindValue(0, st.sensor.paramId);
  query.exec();
  if (query.next()) {
    param_min = query.value(0).toFloat();
    param_max = query.value(1).toFloat();
    have_max  = have_min = true;
    have_high = have_low = false;
  }
}
