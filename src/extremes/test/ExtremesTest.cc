
#include "extremes/ExtremesFilter.hh"

#include "common/KvMetaDataBuffer.hh"
#include "common/KvServiceHelper.hh"
#include "common/TimeBuffer.hh"

#define LOAD_DECL_ONLY
#include "load_17000_20141002.cc"

TEST(ExtremesTest, Filter)
{
  FakeKvApp fa(false); // no threading
  KvServiceHelper kvsh;
  KvMetaDataBuffer kvmdbuf;
  kvmdbuf.setHandler(fa.obsAccess()->handler());

  load_17000_20141002(fa);
  KvMetaDataBuffer::instance()->reload();

  ExtremesFilter_p ef(new ExtremesFilter(211, 5));
  TimeBuffer_p b = boost::make_shared<TimeBuffer>(Sensor_s(), t_17000_20141002(), ef);
  b->syncRequest(FakeKvApp::app()->obsAccess());

  ASSERT_EQ(5, b->size());
}
