
#include "lru_cache.hh"

#include <gtest/gtest.h>

TEST(LruCacheTest, Basic)
{
  lru_cache<int, int> cache(2);
  EXPECT_FALSE(cache.has(5));

  cache.put(2, 12);
  ASSERT_TRUE(cache.has(2));
  EXPECT_EQ(12, cache.get(2));

  cache.put(3, 13);
  ASSERT_TRUE(cache.has(2));
  ASSERT_TRUE(cache.has(3));
  EXPECT_EQ(13, cache.get(3));
  EXPECT_EQ(12, cache.get(2));

  // key 2 at front, key 3 should be evicted
  cache.put(4, 14);
  ASSERT_TRUE(cache.has(2));
  ASSERT_TRUE(cache.has(4));
  ASSERT_FALSE(cache.has(3));
  EXPECT_EQ(14, cache.get(4));
  EXPECT_EQ(12, cache.get(2));
}
