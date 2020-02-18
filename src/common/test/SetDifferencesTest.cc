
#include "set_differences.hh"

#include <gtest/gtest.h>

typedef std::set<int> int_s;
typedef std::insert_iterator<int_s> int_si;

// ========================================================================

#define SET_DIFFERENCES                                                 \
  const int_s input1(values1, values1 + (sizeof(values1)/sizeof(values1[0]))); \
  const int_s input2(values2, values2 + (sizeof(values2)/sizeof(values2[0]))); \
  int_s result1, result2;                                               \
  int_si insert1(result1, result1.begin()), insert2(result2, result2.begin()); \
  set_differences(input1.begin(), input1.end(), input2.begin(), input2.end(), \
      insert1, insert2, std::less<int>());

#define SET_DIFFERENCES3                                                \
  const int_s input1(values1, values1 + (sizeof(values1)/sizeof(values1[0]))); \
  const int_s input2(values2, values2 + (sizeof(values2)/sizeof(values2[0]))); \
  int_s result1, result2, result3;                                      \
  int_si insert1(result1, result1.begin()), insert2(result2, result2.begin()), insert3(result3, result3.begin()); \
  set_differences(input1.begin(), input1.end(), input2.begin(), input2.end(), \
      insert1, insert2, insert3, std::less<int>());

// ========================================================================

TEST(SetDifferencesTest, Disjoint)
{
  const int values1[] = { 1, 3, 5, 7 }, values2[] = { 0, 2, 4, 6 };
  SET_DIFFERENCES;

  EXPECT_EQ(input1, result1);
  EXPECT_EQ(input2, result2);
}

TEST(SetDifferencesTest, Superset1)
{
  const int values1[] = { 1, 3, 5, 7 }, values2[] = { 1, 3, 7 };
  SET_DIFFERENCES;

  EXPECT_EQ(1u, result1.size());
  EXPECT_TRUE(result1.count(5));
  EXPECT_TRUE(result2.empty());
}

TEST(SetDifferencesTest, Superset2)
{
  const int values1[] = { 1, 5, 7 }, values2[] = { 1, 3, 5, 7 };
  SET_DIFFERENCES;

  EXPECT_TRUE(result1.empty());
  EXPECT_EQ(1u, result2.size());
  EXPECT_TRUE(result2.count(3));
}

TEST(SetDifferencesTest, Mixed)
{
  const int values1[] = { 1, 5, 7 }, values2[] = { 1, 3, 5 };
  SET_DIFFERENCES;

  EXPECT_EQ(1u, result1.size());
  EXPECT_TRUE(result1.count(7));

  EXPECT_EQ(1u, result2.size());
  EXPECT_TRUE(result2.count(3));
}

TEST(SetDifferencesTest, Mixed3)
{
  const int values1[] = { 1, 5, 7 }, values2[] = { 1, 3, 5 };
  SET_DIFFERENCES3;

  EXPECT_EQ(1u, result1.size());
  EXPECT_TRUE(result1.count(7));

  EXPECT_EQ(1u, result2.size());
  EXPECT_TRUE(result2.count(3));

  EXPECT_EQ(2u, result3.size());
  EXPECT_TRUE(result3.count(1));
  EXPECT_TRUE(result3.count(5));
}
