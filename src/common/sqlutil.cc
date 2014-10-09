
#include "sqlutil.hh"

void set2sql(std::ostream& sql, const std::set<int>& s)
{
  if (s.size() == 1) {
    sql << " = " << *s.begin();
  } else {
    std::set<int>::const_iterator it = s.begin(), end = s.end();
    sql << " IN (" << *it++;
    for (; it != end; ++it)
      sql << ',' << *it;
    sql << ')';
  }
}

std::ostream& operator<<(std::ostream& sql, const Time2Sql& t)
{
  if (t.value.is_not_a_date_time())
    sql << "NULL";
  else
    sql << '\'' << timeutil::to_iso_extended_string(t.value) << '\'';
  return sql;
}

static QString timeQString(const timeutil::ptime& t)
{
  return QString::fromStdString(timeutil::to_iso_extended_string(t));
}

QString timespan2sql(const TimeSpan& t)
{
  QString sql;
  const bool not0 = t.t0().is_not_a_date_time(), not1 = t.t1().is_not_a_date_time();
  if (not0 and not1)
    return "(0==0)";
  else if (not0)
    return QString("< \'%s\'").arg(timeQString(t.t1()));
  else if (not1)
    return QString("> \'%s\'").arg(timeQString(t.t0()));
  else
    return QString("BETWEEN '%1' AND '%2'")
        .arg(timeQString(t.t0()))
        .arg(timeQString(t.t1()));
}

void sensor2sql(std::ostream& sql, const Sensor& s, const std::string& data_alias, bool model_data)
{
  sql << data_alias << "stationid = " << s.stationId
      << " AND " << data_alias << "paramid = " << s.paramId
      << " AND " << data_alias << "level = " << s.level;
  if (not model_data)
    sql << " AND " << data_alias << "typeid = " << s.typeId
        << " AND " << data_alias << "sensor = '" << s.sensor << "'";
}

void sensors2sql(std::ostream& sql, const Sensor_s& s, const std::string& data_alias, bool model_data)
{
  sql << '(';
  bool haveSensor = false;
  for (Sensor_s::const_iterator it=s.begin(); it!=s.end(); ++it) {
    if (not it->valid())
      continue;
    if (haveSensor)
      sql << " OR ";
    haveSensor = true;
    sql << '(';
    sensor2sql(sql, *it, data_alias, model_data);
    sql << ')';
  }
  if (not haveSensor)
    sql << "0=0";
  sql << ')';
}

QString sensor2sql(const Sensor& s, const QString& data_alias, bool model_data)
{
  std::ostringstream sqls;
  sensor2sql(sqls, s, data_alias.toStdString(), model_data);
  return QString::fromStdString(sqls.str());
}

QString sensors2sql(const Sensor_s& s, const QString& data_alias, bool model_data)
{
  std::ostringstream sqls;
  sensors2sql(sqls, s, data_alias.toStdString(), model_data);
  return QString::fromStdString(sqls.str());
}

void sensortime2sql(std::ostream& sql, const SensorTime& st, const std::string& table_alias, bool model_data)
{
  sql << '(';
  sensor2sql(sql, st.sensor, table_alias, model_data);
  sql << " AND " << table_alias << "obstime = " << time2sql(st.time) << ')';
}

void sensortimes2sql(std::ostream& sql, const SensorTime_v& s, const std::string& table_alias, bool model_data)
{
  if (s.empty()) {
    sql << "(0=0)";
  } else {
    sql << '(';
    for (SensorTime_v::const_iterator it=s.begin(); it!=s.end(); ++it) {
      if (it != s.begin())
        sql << " OR ";
      sensortime2sql(sql, *it, table_alias, model_data);
    }
    sql << ')';
  }
}
