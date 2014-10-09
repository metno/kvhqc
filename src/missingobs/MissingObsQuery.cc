
#include "MissingObsQuery.hh"

#include "common/KvHelpers.hh"
#include "common/sqlutil.hh"

#define MILOGGER_CATEGORY "kvhqc.MissingTableModel"
#include "common/ObsLogging.hh"

namespace /*anonymous*/ {

Time my_qsql_time(const std::string& s)
{
  return timeutil::from_iso_extended_string(s);
}

bool initMetaType = false;

} // namespace anonymous

MissingObsQuery::MissingObsQuery(const TimeSpan& time, const hqc::int_s& typeids, size_t priority)
  : SignalTask(priority)
  , mTime(time)
  , mTypeIds(typeids)
{
  if (not initMetaType) {
    qRegisterMetaType<SensorTime>("SensorTime");
    qRegisterMetaType<SensorTime_v>("SensorTime_v");
    initMetaType = true;
  }
}

QString MissingObsQuery::querySql(QString dbversion) const
{
  const bool sqlite = dbversion.contains("d=sqlite");

  QString timevalues = "(";
  if (not sqlite)
    timevalues += "VALUES ";

  bool day1 = true;
  const timeutil::pdate d0 = mTime.t0().date(), d1 = mTime.t1().date();
  for (timeutil::pdate d = d0; d <= d1; d += boost::gregorian::days(1)) {
    const QString t6 = "'" + timeQString(timeutil::ptime(d, boost::posix_time::hours(6))) + "'";
    const QString t7 = "'" + timeQString(timeutil::ptime(d, boost::posix_time::hours(7))) + "'";

    if (sqlite) {
      if (not day1)
        timevalues += " UNION ";
      timevalues += "SELECT " + t6;
      if (day1)
        timevalues += " AS ts";
      timevalues += ", 6";
      if (day1)
        timevalues += " AS hr";
      timevalues += " UNION SELECT " + t7 + ", 7";
    } else {
      if (not day1)
        timevalues += ", ";
      timevalues += "(timestamp " + t6 + ", 6), (timestamp " + t7 + ", 7)";
    }
    day1 = false;
  }
  timevalues += ") AS times";
  if (not sqlite)
    timevalues += "(ts, hr)";

  const QString bool_TRUE(sqlite ? "1" : "TRUE");
  QString sql = "SELECT o.stationid, o.typeid, times.ts FROM obs_pgm AS o, " + timevalues +
      " WHERE " + QString::fromStdString(Helpers::isNorwegianStationIdSQL("o.stationid")) +
      " AND " + set2sql("o.typeid", mTypeIds) +
      " AND o.paramid = 110"
      " AND o.fromtime <= times.ts AND (o.totime IS NULL OR o.totime >= times.ts)"
      " AND    ((times.hr = 6 AND o.kl06 = " + bool_TRUE + ")"
      "      OR (times.hr = 7 AND o.kl07 = " + bool_TRUE + "))"
      " AND NOT EXISTS (SELECT * FROM data AS d"
      "                  WHERE d.stationid = o.stationid"
      "                    AND d.typeid = o.typeid"
      "                    AND d.paramid = o.paramid"
      "                    AND d.level = 0 AND d.sensor = '0'" // FIXME which level/sensor to use? (see below)
      "                    AND d.obstime = times.ts"
      "                    AND NOT (substr(d.controlinfo, 7,1) IN ('1','2','3')" // fmis == 1, 2, 3 (original or corrected missing)
      "                         AND substr(d.controlinfo,13,1) IN ('0','1')))"   // fd <= 1 (no accumulation)
      " ORDER BY o.stationid, o.typeid, times.ts";
  return sql;
}

void MissingObsQuery::notifyRow(const ResultRow& row)
{
  const int stationId = row.getInt(0);
  const int typeId    = row.getInt(1);
  const Time time     = my_qsql_time(row.getStdString(2));

  // FIXME which level/sensor to use? (see above)
  mMissing.push_back(SensorTime(Sensor(stationId, kvalobs::PARAMID_RR_24, /*level*/0, /*sensor*/0, typeId), time));
}

void MissingObsQuery::notifyStatus(int status)
{
  METLIBS_LOG_SCOPE(LOGVAL(status));
  if (status > COMPLETE)
    mMissing.clear();
  SignalTask::notifyStatus(status);
}
