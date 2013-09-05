
#define VERSIONEDVALUE_TEST 1
#include "VersionedValue.hh"
#undef VERSIONEDVALUE_TEST

#include <gtest/gtest.h>

TEST(VersionedValueTest, BasicInt)
{
    typedef VersionedValue<int> VVint;
    const int V1 = 17, V2 = 12, V3 = 22;

    VVint v(V1);
    EXPECT_FALSE(v.modified());
    EXPECT_EQ(1, v.mVersions.size()); ASSERT_EQ(0, v.mCurrent); EXPECT_EQ(0, v.mVersions[0].version); EXPECT_EQ(V1, v.mVersions[0].value);

    EXPECT_FALSE(v.setVersion(1, true));
    EXPECT_EQ(1, v.mVersions.size()); ASSERT_EQ(0, v.mCurrent); EXPECT_EQ(0, v.mVersions[0].version); EXPECT_EQ(V1, v.mVersions[0].value);

    EXPECT_TRUE(v.setValue(1, V2));
    EXPECT_EQ(2, v.mVersions.size()); ASSERT_EQ(1, v.mCurrent); EXPECT_EQ(1, v.mVersions[1].version); EXPECT_EQ(V2, v.mVersions[1].value);

    EXPECT_TRUE(v.modified());
    EXPECT_EQ(V2, v.value());

    EXPECT_FALSE(v.setVersion(2, true));
    EXPECT_EQ(2, v.mVersions.size()); ASSERT_EQ(1, v.mCurrent); EXPECT_EQ(1, v.mVersions[1].version); EXPECT_EQ(V2, v.mVersions[1].value);

    EXPECT_FALSE(v.setVersion(3, true));
    EXPECT_EQ(2, v.mVersions.size()); ASSERT_EQ(1, v.mCurrent); EXPECT_EQ(1, v.mVersions[1].version); EXPECT_EQ(V2, v.mVersions[1].value);

    EXPECT_TRUE(v.setValue(3, V1));
    EXPECT_EQ(3, v.mVersions.size()); ASSERT_EQ(2, v.mCurrent); EXPECT_EQ(3, v.mVersions[2].version); EXPECT_EQ(V1, v.mVersions[2].value);

    EXPECT_FALSE(v.modified());
    EXPECT_EQ(V1, v.value());
    
    std::cout << "undo" << std::endl;
    EXPECT_TRUE(v.setVersion(2, true));
    EXPECT_EQ(2, v.mVersions.size()); ASSERT_EQ(1, v.mCurrent); EXPECT_EQ(1, v.mVersions[1].version); EXPECT_EQ(V2, v.mVersions[1].value);
    EXPECT_TRUE(v.modified());
    EXPECT_EQ(V2, v.value());

    std::cout << "set for version 2" << std::endl;
    EXPECT_TRUE(v.setValue(2, V3));
    EXPECT_EQ(3, v.mVersions.size()); ASSERT_EQ(2, v.mCurrent); EXPECT_EQ(2, v.mVersions[2].version); EXPECT_EQ(V3, v.mVersions[2].value);
    EXPECT_EQ(V3, v.value());

    std::cout << "undo again" << std::endl;
    EXPECT_TRUE(v.setVersion(1, true));
    EXPECT_EQ(2, v.mVersions.size()); ASSERT_EQ(1, v.mCurrent); EXPECT_EQ(1, v.mVersions[1].version); EXPECT_EQ(V2, v.mVersions[1].value);

    EXPECT_TRUE(v.reset(V3));
    EXPECT_EQ(1, v.mVersions.size()); ASSERT_EQ(0, v.mCurrent); EXPECT_EQ(0, v.mVersions[0].version); EXPECT_EQ(V3, v.mVersions[0].value);
}
