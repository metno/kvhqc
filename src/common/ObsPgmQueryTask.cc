
#include "ObsPgmQueryTask.hh"

#include "common/sqlutil.hh"

#define MILOGGER_CATEGORY "kvhqc.ObsPgmQueryTask"
#include "common/ObsLogging.hh"

namespace /*anonymous*/ {

bool initMetaType = false;

} // namespace anonymous

LOG_CONSTRUCT_COUNTER;

ObsPgmQueryTask::ObsPgmQueryTask(const int_s& stationIds, size_t priority)
  : QueryTask(priority)
  , mStationIds(stationIds)
{
  METLIBS_LOG_SCOPE();
  LOG_CONSTRUCT();
  if (not initMetaType) {
    qRegisterMetaType<hqc::hqcObsPgm_v>("hqcObsPgm_v");
    initMetaType = true;
  }
}

ObsPgmQueryTask::~ObsPgmQueryTask()
{
  METLIBS_LOG_SCOPE();
  LOG_DESTRUCT();
}

QString ObsPgmQueryTask::querySql(QString dbversion) const
{
  return QString("SELECT o.stationid, o.paramid, o.level, o.nr_sensor, o.typeid,"
                 " o.priority_message, o.collector, "
                 " o.kl00, o.kl01, o.kl02, o.kl03, o.kl04, o.kl05,"
                 " o.kl06, o.kl07, o.kl08, o.kl09, o.kl10, o.kl11,"
                 " o.kl12, o.kl13, o.kl14, o.kl15, o.kl16, o.kl17,"
                 " o.kl18, o.kl19, o.kl20, o.kl21, o.kl22, o.kl23,"
                 " o.mon, o.tue, o.wed, o.thu, o.fri, o.sat, o.sun,"
                 " o.fromtime, o.totime"
                 " FROM obs_pgm o"
                 " WHERE ") +
         set2sql("o.stationid", mStationIds);
}

void ObsPgmQueryTask::notifyRow(const ResultRow& row)
{
  METLIBS_LOG_SCOPE();
  int col = 0;

  const int stationid = row.getInt(col++);
  const int paramid = row.getInt(col++);
  const int level = row.getInt(col++);
  const int nr_sensor = row.getInt(col++);
  const int msg_fmt_id = row.getInt(col++);
  const bool priority_message = row.getBool(col++);
  const bool collector = row.getBool(col++);
  const bool kl00 = row.getBool(col++);
  const bool kl01 = row.getBool(col++);
  const bool kl02 = row.getBool(col++);
  const bool kl03 = row.getBool(col++);
  const bool kl04 = row.getBool(col++);
  const bool kl05 = row.getBool(col++);
  const bool kl06 = row.getBool(col++);
  const bool kl07 = row.getBool(col++);
  const bool kl08 = row.getBool(col++);
  const bool kl09 = row.getBool(col++);
  const bool kl10 = row.getBool(col++);
  const bool kl11 = row.getBool(col++);
  const bool kl12 = row.getBool(col++);
  const bool kl13 = row.getBool(col++);
  const bool kl14 = row.getBool(col++);
  const bool kl15 = row.getBool(col++);
  const bool kl16 = row.getBool(col++);
  const bool kl17 = row.getBool(col++);
  const bool kl18 = row.getBool(col++);
  const bool kl19 = row.getBool(col++);
  const bool kl20 = row.getBool(col++);
  const bool kl21 = row.getBool(col++);
  const bool kl22 = row.getBool(col++);
  const bool kl23 = row.getBool(col++);
  const bool mon = row.getBool(col++);
  const bool tue = row.getBool(col++);
  const bool wed = row.getBool(col++);
  const bool thu = row.getBool(col++);
  const bool fri = row.getBool(col++);
  const bool sat = row.getBool(col++);
  const bool sun = row.getBool(col++);
  const timeutil::ptime fromtime = timeutil::from_iso_extended_string(row.getStdString(col++));
  const timeutil::ptime totime = timeutil::from_iso_extended_string(row.getStdString(col++));

  mObsPgms.push_back(hqc::hqcObsPgm(stationid, paramid, level, nr_sensor, msg_fmt_id, priority_message, collector, kl00, kl01, kl02, kl03, kl04, kl05, kl06,
                                    kl07, kl08, kl09, kl10, kl11, kl12, kl13, kl14, kl15, kl16, kl17, kl18, kl19, kl20, kl21, kl22, kl23, mon, tue, wed, thu,
                                    fri, sat, sun, fromtime, totime));
}
