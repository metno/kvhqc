
#define VERSIONEDVALUE_TEST 1
#include "VersionedValue.hh"
#undef VERSIONEDVALUE_TEST

#include <gtest/gtest.h>

#define CHECK_STATE(vvsize, vvcurrent, vvversion, vvvalue)              \
    EXPECT_EQ(vvsize, v.mVersions.size());                              \
    ASSERT_EQ(vvcurrent, v.mCurrent);                                   \
    EXPECT_EQ(vvversion, v.mVersions[vvcurrent].version);               \
    EXPECT_EQ(vvvalue, v.mVersions[vvcurrent].value);

TEST(VersionedValueTest, BasicInt)
{
    typedef VersionedValue<int> VVint;
    const int V1 = 17, V2 = 12, V3 = 22;

    VVint v(V1);
    EXPECT_FALSE(v.modified());
    CHECK_STATE(1, 0, 0, V1);

    EXPECT_FALSE(v.setVersion(1, true));
    CHECK_STATE(1, 0, 0, V1);

    EXPECT_TRUE(v.setValue(1, V2));
    CHECK_STATE(2, 1, 1, V2);
    EXPECT_TRUE(v.modified());
    EXPECT_EQ(V2, v.value());

    EXPECT_FALSE(v.setVersion(2, true));
    CHECK_STATE(2, 1, 1, V2);

    EXPECT_FALSE(v.setVersion(3, true));
    CHECK_STATE(2, 1, 1, V2);

    EXPECT_TRUE(v.setValue(3, V1));
    CHECK_STATE(3, 2, 3, V1);
    EXPECT_FALSE(v.modified());
    EXPECT_EQ(V1, v.value());
    
    EXPECT_TRUE(v.setVersion(2, false));
    CHECK_STATE(3, 1, 1, V2);
    EXPECT_TRUE(v.modified());
    EXPECT_EQ(V2, v.value());

    EXPECT_TRUE(v.setValue(2, V3));
    CHECK_STATE(3, 2, 2, V3);
    EXPECT_EQ(V3, v.value());

    EXPECT_TRUE(v.setVersion(1, false));
    CHECK_STATE(3, 1, 1, V2);

    EXPECT_TRUE(v.reset(V3));
    CHECK_STATE(1, 0, 0, V3);
}
