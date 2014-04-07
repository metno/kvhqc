
#include "CachingAccess.hh"
#include "SqliteAccess.hh"
#include "TimeBuffer.hh"

#include "util/make_set.hh"

#include <gtest/gtest.h>

namespace /*anonymous*/ {

inline timeutil::ptime s2t(const std::string& t)
{ return timeutil::from_iso_extended_string(t); }

class CountingBuffer : public TimeBuffer
{
public:
  CountingBuffer(const Sensor_s& sensors, const TimeSpan& timeSpan, ObsFilter_p filter = ObsFilter_p())
    : TimeBuffer(sensors, timeSpan, filter), countComplete(0), countNew(0), countUpdate(0) { }

  CountingBuffer(const Sensor& sensor, const TimeSpan& timeSpan, ObsFilter_p filter = ObsFilter_p())
    : TimeBuffer(make_set<Sensor_s>(sensor), timeSpan, filter), countComplete(0), countNew(0), countUpdate(0) { }

  virtual void completed(bool failed)
    { TimeBuffer::completed(failed); countComplete += 1; }

  virtual void onNewData(const ObsData_pv& data)
    { TimeBuffer::onNewData(data); countNew += 1; }

  virtual void onUpdateData(const ObsData_pv& data)
    { TimeBuffer::onUpdateData(data); countUpdate += 1; }

  size_t countComplete, countNew, countUpdate;
};

HQC_TYPEDEF_P(CountingBuffer);

} // namespace anonymous

// ========================================================================

TEST(CachingAccessTest, Single)
{
  SqliteAccess_p sqla(new SqliteAccess);
  sqla->insertDataFromFile(std::string(TEST_SOURCE_DIR)+"/data_18210_20130410.txt");
  CachingAccess_p ca(new CachingAccess(sqla));

  const Sensor sensor(18210, 211, 0, 0, 514);

  // this assumes that SqliteAccess is syncronous
  { const TimeSpan time(s2t("2013-04-01 00:00:00"), s2t("2013-04-03 00:00:00"));
    CountingBuffer_p counter(new CountingBuffer(sensor, time));
    counter->postRequest(ca);
    EXPECT_EQ(1, sqla->countPost());
    EXPECT_EQ(1, counter->countNew);
    EXPECT_EQ(2*24 + 1, counter->size());
    EXPECT_EQ(1, counter->countComplete);
  }

  { const TimeSpan time(s2t("2013-04-01 00:00:00"), s2t("2013-04-02 00:00:00"));
    CountingBuffer_p counter(new CountingBuffer(sensor, time));
    counter->postRequest(ca);
    EXPECT_EQ(1, sqla->countPost());
    EXPECT_EQ(1, counter->countNew);
    EXPECT_EQ(1*24 + 1, counter->size());
    EXPECT_EQ(1, counter->countComplete);
  }

  { const TimeSpan time(s2t("2013-04-02 00:00:00"), s2t("2013-04-03 00:00:00"));
    CountingBuffer_p counter(new CountingBuffer(sensor, time));
    counter->postRequest(ca);
    EXPECT_EQ(1, sqla->countPost());
    EXPECT_EQ(1*24 + 1, counter->size());
  }

  { const TimeSpan time(s2t("2013-04-01 06:00:00"), s2t("2013-04-03 06:00:00"));
    CountingBuffer_p counter(new CountingBuffer(sensor, time));
    counter->postRequest(ca);
    EXPECT_EQ(2, sqla->countPost());
    EXPECT_EQ(2*24 + 1, counter->size());
  }

  ca->cleanCache(timeutil::now() + boost::posix_time::hours(1));

  { const TimeSpan time(s2t("2013-04-01 06:00:00"), s2t("2013-04-02 06:00:00"));
    CountingBuffer_p counter(new CountingBuffer(sensor, time));
    counter->postRequest(ca);
    EXPECT_EQ(3, sqla->countPost());
    EXPECT_EQ(1*24 + 1, counter->size());
  }
  { const TimeSpan time(s2t("2013-04-01 06:00:00"), s2t("2013-04-03 06:00:00"));
    CountingBuffer_p counter(new CountingBuffer(sensor, time));
    counter->postRequest(ca);
    EXPECT_EQ(4, sqla->countPost());
    EXPECT_EQ(2*24 + 1, counter->size());
  }
}

TEST(CachingAccessTest, Multi)
{
  SqliteAccess_p sqla(new SqliteAccess);
  sqla->insertDataFromFile(std::string(TEST_SOURCE_DIR)+"/data_18700_20140304.txt");
  CachingAccess_p ca(new CachingAccess(sqla));

  const Sensor sensor1(18210, 211, 0, 0, 514);
  const Sensor sensor2(18700, 211, 0, 0, 330);
  const Sensor_s sensors = (SetMaker<Sensor_s>() << sensor1 << sensor2).set();

  // this assumes that SqliteAccess is syncronous
  { const TimeSpan time(s2t("2014-03-01 00:00:00"), s2t("2014-03-01 06:00:00"));
    CountingBuffer_p counter(new CountingBuffer(sensor1, time));
    counter->postRequest(ca);
    EXPECT_EQ(1, sqla->countPost());
    EXPECT_EQ(1, counter->countNew);
    EXPECT_EQ(7, counter->size());
    EXPECT_EQ(1, counter->countComplete);
  }

  { const TimeSpan time(s2t("2014-03-01 03:00:00"), s2t("2014-03-01 09:00:00"));
    CountingBuffer_p counter(new CountingBuffer(sensor2, time));
    counter->postRequest(ca);
    EXPECT_EQ(2, sqla->countPost());
    EXPECT_EQ(1, counter->countNew);
    EXPECT_EQ(7, counter->size());
    EXPECT_EQ(1, counter->countComplete);
  }

  { const TimeSpan time(s2t("2014-03-01 00:00:00"), s2t("2014-03-01 09:00:00"));
    CountingBuffer_p counter(new CountingBuffer(sensors, time));
    counter->postRequest(ca);
    EXPECT_EQ(4, sqla->countPost());
    EXPECT_EQ(20, counter->size());
    EXPECT_EQ(1, counter->countComplete);
  }

  ca->cleanCache(timeutil::now() + boost::posix_time::hours(1));

  { const TimeSpan time(s2t("2014-03-01 00:00:00"), s2t("2014-03-01 09:00:00"));
    CountingBuffer_p counter(new CountingBuffer(sensors, time));
    counter->postRequest(ca);
    EXPECT_EQ(5, sqla->countPost());
    EXPECT_EQ(20, counter->size());
    EXPECT_EQ(1, counter->countComplete);
  }
}
