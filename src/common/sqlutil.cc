
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
  if (s.empty()) {
    sql << "(0=0)";
  } else {
    sql << '(';
    for (Sensor_s::const_iterator it=s.begin(); it!=s.end(); ++it) {
      if (it != s.begin())
        sql << " OR ";
      sql << '(';
      sensor2sql(sql, *it, data_alias, model_data);
      sql << ')';
    }
    sql << ')';
  }
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
