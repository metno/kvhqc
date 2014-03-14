
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
  sql << '\'' << timeutil::to_iso_extended_string(t.value) << '\'';
  return sql;
}

void sensor2sql(std::ostream& sql, const Sensor& s, const std::string& data_alias)
{
  sql << data_alias << "stationid = " << s.stationId
      << " AND " << data_alias << "paramid = " << s.paramId
      << " AND " << data_alias << "typeid = " << s.typeId
      << " AND " << data_alias << "level = " << s.level
      << " AND " << data_alias << "sensor = " << s.sensor;
}

void sensors2sql(std::ostream& sql, const Sensor_s& s, const std::string& data_alias)
{
  if (s.empty()) {
    sql << "(0=0)";
  } else {
    sql << '(';
    for (Sensor_s::const_iterator it=s.begin(); it!=s.end(); ++it) {
      if (it != s.begin())
        sql << " OR ";
      sql << '(';
      sensor2sql(sql, *it, data_alias);
      sql << ')';
    }
    sql << ')';
  }
}
