
#include "missingobs/MissingObsQuery.hh"

#include "common/CachingAccess.hh"
#include "common/KvHelpers.hh"
#include "common/KvMetaDataBuffer.hh"
#include "common/KvServiceHelper.hh"
#include "common/TimeBuffer.hh"

#include <QRegExp>

#define LOAD_DECL_ONLY
#include "load_12600_20141008.cc"
#include "load_17150_20141008.cc"

#define MILOGGER_CATEGORY "kvhqc.test.MissingObsTest"
#include "common/ObsLogging.hh"

static inline const Sensor& s(ObsData_p obs) { return obs->sensorTime().sensor; }
static inline const std::string t(ObsData_p obs) { return timeutil::to_iso_extended_string(obs->sensorTime().time); }
static inline float cv(ObsData_p obs) { return obs->corrected(); }

static const QRegExp re_collapse_ws("[\\s\\n]+");
static inline std::string collapse_std(QString a)
{ return QString(a).replace(re_collapse_ws, " ").toStdString(); }

TEST(MissingObsTest, SqlStatement)
{
  hqc::int_s typeids;
  typeids.insert(302);
  MissingObsQuery q(TimeSpan(s2t("2014-10-01 06:00"),s2t("2014-10-02 06:00")),
      typeids, QueryTask::PRIORITY_AUTOMATIC);

  // this is far from black box testing
  { 
    QString actual = q.querySql("d=sqlite");
    QString expected = "SELECT o.stationid, o.typeid, times.ts FROM obs_pgm AS o,"
        " (SELECT '2014-10-01 06:00:00' AS ts, 6 AS hr UNION SELECT '2014-10-01 07:00:00', 7"
        "   UNION SELECT '2014-10-02 06:00:00', 6 UNION SELECT '2014-10-02 07:00:00', 7) AS times"
        " WHERE (o.stationid BETWEEN 60 AND 99999)"
        "   AND o.typeid = 302 AND o.paramid = 110"
        "   AND o.fromtime <= times.ts"
        "   AND (o.totime IS NULL OR o.totime >= times.ts)"
        "   AND ((times.hr = 6 AND o.kl06 = 1)"
        "     OR (times.hr = 7 AND o.kl07 = 1))"
        "   AND NOT EXISTS (SELECT * FROM data AS d"
        "                    WHERE d.stationid = o.stationid"
        "                      AND d.typeid = o.typeid"
        "                      AND d.paramid = o.paramid"
        "                      AND d.level = 0 AND d.sensor = '0'"
        "                      AND d.obstime = times.ts"
        "                      AND NOT (substr(d.controlinfo, 7,1) IN ('1','2','3')"
        "                           AND substr(d.controlinfo,13,1) IN ('0','1')))"
        " ORDER BY o.stationid, o.typeid, times.ts";
    EXPECT_EQ(collapse_std(expected), collapse_std(actual));
  }
  {
    QString actual = q.querySql("d=postgresql");
    QString expected = "SELECT o.stationid, o.typeid, times.ts FROM obs_pgm AS o,"
        " (VALUES (timestamp '2014-10-01 06:00:00', 6), (timestamp '2014-10-01 07:00:00', 7),"
        "         (timestamp '2014-10-02 06:00:00', 6), (timestamp '2014-10-02 07:00:00', 7)) AS times(ts, hr)"
        " WHERE (o.stationid BETWEEN 60 AND 99999)"
        "   AND o.typeid = 302 AND o.paramid = 110"
        "   AND o.fromtime <= times.ts"
        "   AND (o.totime IS NULL OR o.totime >= times.ts)"
        "   AND ((times.hr = 6 AND o.kl06 = TRUE)"
        "     OR (times.hr = 7 AND o.kl07 = TRUE))"
        "   AND NOT EXISTS (SELECT * FROM data AS d"
        "                    WHERE d.stationid = o.stationid"
        "                      AND d.typeid = o.typeid"
        "                      AND d.paramid = o.paramid"
        "                      AND d.level = 0 AND d.sensor = '0'"
        "                      AND d.obstime = times.ts"
        "                      AND NOT (substr(d.controlinfo, 7,1) IN ('1','2','3')"
        "                           AND substr(d.controlinfo,13,1) IN ('0','1')))"
        " ORDER BY o.stationid, o.typeid, times.ts";
    EXPECT_EQ(collapse_std(expected), collapse_std(actual));
  }
}

TEST(MissingObsTest, Query)
{
  std::shared_ptr<FakeKvApp> fa(std::make_shared<FakeKvApp>(false)); // no threading
  KvServiceHelper kvsh(fa);
  KvMetaDataBuffer kvmdbuf;
  kvmdbuf.setHandler(fa->obsAccess()->handler());

  load_12600_20141008(*fa);
  KvMetaDataBuffer::instance()->reload();

  hqc::int_s typeids;
  typeids.insert(302);
  std::unique_ptr<MissingObsQuery> q(new MissingObsQuery(t_12600_20141008(), typeids, QueryTask::PRIORITY_AUTOMATIC));

  fa->obsAccess()->handler()->postTask(q.get());

  ASSERT_EQ(2, q->missing().size());
}
