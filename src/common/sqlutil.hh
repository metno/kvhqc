
#ifndef SQLUTIL_HH
#define SQLUTIL_HH 1

#include "util/timeutil.hh"
#include "common/Sensor.hh"

#include <iosfwd>
#include <set>

void set2sql(std::ostream& sql, const std::set<int>& s);

struct Time2Sql {
  Time2Sql(const timeutil::ptime& v) : value(v) { }
  const timeutil::ptime& value;
};
std::ostream& operator<<(std::ostream& sql, const Time2Sql& t);

inline Time2Sql time2sql(const timeutil::ptime& t)
  { return Time2Sql(t); }

void sensor2sql(std::ostream& sql, const Sensor& s, const std::string& data_alias);

void sensors2sql(std::ostream& sql, const Sensor_s& s, const std::string& data_alias);

#endif // SQLUTIL_HH