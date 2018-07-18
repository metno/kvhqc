/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2018 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: kvalobs-dev@met.no

  This file is part of HQC

  HQC is free software; you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  HQC is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General Public License
  along with HQC; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/


#include "CachedParamLimits.hh"

#include "HqcApplication.hh"
#include "HqcSystemDB.hh"
#include "KvHelpers.hh"
#include "StationParamSQLTask.hh"
#include "SyncTask.hh"

#include <kvcpp/KvApp.h>
#include <kvalobs/kvStationParam.h>

#include <QSqlQuery>
#include <QVariant>

#define MILOGGER_CATEGORY "kvhqc.CachedParamLimits"
#include "common/ObsLogging.hh"

//#define DEBUG_ME_ON 1
#ifdef DEBUG_ME_ON
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

CachedParamLimits::Limits::Limits()
{
  have_max = have_high = have_low = have_min = false;
  fromtime = totime = timeutil::ptime();
}

CachedParamLimits::ParamLimit CachedParamLimits::check(const SensorTime& st, float value)
{
  DEBUG_ME(METLIBS_LOG_SCOPE(LOGVAL(st) << LOGVAL(value)));
  if (value == kvalobs::MISSING or value == kvalobs::REJECTED)
    return Ok;
  if (!st.valid()) {
    METLIBS_LOG_ERROR("param limits check for invalid sensortime " << st);
    return OutsideMinMax;
  }

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
    const timeutil::ptime& ft = task.fromtimes().at(i);
    if (md.find("max") == std::string::npos || md.find("high") == std::string::npos)
      continue;
    if (ft <= st.time and (fromtime.is_not_a_date_time() or fromtime < ft)) {
      metadata = QString::fromStdString(md);
      fromtime = ft;
    }
    if (ft > st.time and (totime.is_not_a_date_time() or totime > ft)) {
      totime = ft;
    }
  }

  return metadata;
}

void CachedParamLimits::parseMetadata(const QString& metadata, Limits& limits)
{

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
