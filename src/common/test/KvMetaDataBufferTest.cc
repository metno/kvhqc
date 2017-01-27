
#include "KvMetaDataBuffer.hh"

#include "FakeKvApp.hh"
#include "TestHelpers.hh"

#define LOAD_DECL_ONLY
#include "load_18700_20141001.cc"

#define MILOGGER_CATEGORY "kvhqc.test.KvMetaDataBufferTest"
#include "util/HqcLogging.hh"

TEST(KvMetaDataBufferTest, Station)
{
  FakeKvApp fa(false); // no threading
  KvMetaDataBuffer kvmdbuf;
  kvmdbuf.setHandler(fa.obsAccess()->handler());

  load_18700_20141001(fa);
  KvMetaDataBuffer::instance()->reload();

  ASSERT_FALSE(kvmdbuf.isKnownStation(18210));
  ASSERT_TRUE (kvmdbuf.isKnownStation(18700));

  try {
    const kvalobs::kvStation s = kvmdbuf.findStation(18700);
    ASSERT_EQ(18700, s.stationID());
  } catch(...) {
    FAIL() << "got an exception";
  }
}

TEST(KvMetaDataBufferTest, Param)
{
  FakeKvApp fa(false); // no threading
  KvMetaDataBuffer kvmdbuf;
  kvmdbuf.setHandler(fa.obsAccess()->handler());

  load_18700_20141001(fa);
  KvMetaDataBuffer::instance()->reload();

  ASSERT_FALSE(kvmdbuf.isKnownParam(83));
  ASSERT_TRUE (kvmdbuf.isKnownParam(81));

  try {
    const kvalobs::kvParam p = kvmdbuf.findParam(81);
    ASSERT_EQ(81, p.paramID());
  } catch(...) {
    FAIL() << "got an exception";
  }

  try {
    ASSERT_FALSE(kvmdbuf.isDirectionInDegreesParam(81));
    ASSERT_TRUE (kvmdbuf.isDirectionInDegreesParam(61));
  } catch(...) {
    FAIL() << "got an exception";
  }
}

TEST(KvMetaDataBufferTest, Type)
{
  FakeKvApp fa(false); // no threading
  KvMetaDataBuffer kvmdbuf;
  kvmdbuf.setHandler(fa.obsAccess()->handler());

  load_18700_20141001(fa);
  KvMetaDataBuffer::instance()->reload();

  ASSERT_FALSE(kvmdbuf.isKnownType(402));
  ASSERT_TRUE (kvmdbuf.isKnownType(308));

  try {
    const kvalobs::kvTypes t = kvmdbuf.findType(308);
    ASSERT_EQ(308, t.typeID());
  } catch(...) {
    FAIL() << "got an exception";
  }
}
