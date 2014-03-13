
#include "CachingAccess.hh"
#include "SqliteAccess.hh"
#include "TimeBuffer.hh"

#include <gtest/gtest.h>

namespace /*anonymous*/ {

inline timeutil::ptime s2t(const std::string& t)
{ return timeutil::from_iso_extended_string(t); }

class CountingBuffer : public TimeBuffer
{
public:
  CountingBuffer(const Sensor& sensor, const TimeSpan& timeSpan, ObsFilter_p filter = ObsFilter_p())
    : TimeBuffer(sensor, timeSpan, filter), countComplete(0), countNew(0), countUpdate(0) { }

  virtual void completed(bool failed)
    { TimeBuffer::completed(failed); countComplete += 1; }

  virtual void newData(const ObsData_pv& data)
    { TimeBuffer::newData(data); countNew += 1; }

  virtual void updateData(const ObsData_pv& data)
    { TimeBuffer::updateData(data); countUpdate += 1; }

  size_t countComplete, countNew, countUpdate;
};

HQC_TYPEDEF_P(CountingBuffer);

} // namespace anonymous

// ========================================================================

TEST(CachingAccessTest, Basic)
{
  SqliteAccess_p sqla(new SqliteAccess);
  sqla->insertDataFromFile(std::string(TEST_SOURCE_DIR)+"/../../common/test/data_18210_20130410.txt");
  CachingAccess_p ca(new CachingAccess(sqla));

  const Sensor sensor(18210, 211, 0, 0, 514);

  // this assumes that SqliteAccess is syncronous
  { const TimeRange time(s2t("2013-04-01 00:00:00"), s2t("2013-04-03 00:00:00"));
    CountingBuffer_p counter(new CountingBuffer(sensor, time));
    counter->postRequest(ca);
    EXPECT_EQ(1, sqla->countPost());
    EXPECT_EQ(2*24 + 1, counter->size());
    EXPECT_EQ(1, counter->countComplete);
  }

  { const TimeRange time(s2t("2013-04-01 00:00:00"), s2t("2013-04-02 00:00:00"));
    CountingBuffer_p counter(new CountingBuffer(sensor, time));
    counter->postRequest(ca);
    EXPECT_EQ(1, sqla->countPost());
    EXPECT_EQ(1*24 + 1, counter->size());
    EXPECT_EQ(1, counter->countComplete);
  }

  { const TimeRange time(s2t("2013-04-02 00:00:00"), s2t("2013-04-03 00:00:00"));
    CountingBuffer_p counter(new CountingBuffer(sensor, time));
    counter->postRequest(ca);
    EXPECT_EQ(1, sqla->countPost());
    EXPECT_EQ(1*24 + 1, counter->size());
  }

  { const TimeRange time(s2t("2013-04-01 06:00:00"), s2t("2013-04-03 06:00:00"));
    CountingBuffer_p counter(new CountingBuffer(sensor, time));
    counter->postRequest(ca);
    EXPECT_EQ(2, sqla->countPost());
    EXPECT_EQ(2*24 + 1, counter->size());
  }

  ca->cleanCache(timeutil::now() + boost::posix_time::hours(1));

  { const TimeRange time(s2t("2013-04-01 06:00:00"), s2t("2013-04-02 06:00:00"));
    CountingBuffer_p counter(new CountingBuffer(sensor, time));
    counter->postRequest(ca);
    EXPECT_EQ(3, sqla->countPost());
    EXPECT_EQ(1*24 + 1, counter->size());
  }
  { const TimeRange time(s2t("2013-04-01 06:00:00"), s2t("2013-04-03 06:00:00"));
    CountingBuffer_p counter(new CountingBuffer(sensor, time));
    counter->postRequest(ca);
    EXPECT_EQ(4, sqla->countPost());
    EXPECT_EQ(2*24 + 1, counter->size());
  }
}
