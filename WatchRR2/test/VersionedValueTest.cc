
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

TEST(VersionedValueTest, Reset)
{
    typedef VersionedValue<int> VVint;
    const int V1 = 17, V2 = 12;

    VVint v(V1);
    EXPECT_FALSE(v.setVersion(1, true));
    EXPECT_FALSE(v.reset(V1));

    EXPECT_TRUE(v.reset(V2));

    EXPECT_FALSE(v.setVersion(1, true));
    EXPECT_TRUE(v.setValue(1, V1));
    EXPECT_TRUE(v.reset(V2));
}

TEST(VersionedValueTest, UndoRedo)
{
    typedef VersionedValue<int> VVint;
    const int V0 = 8, V1 = 17, V2 = 12, V3 = 22;

    VVint v(V0);
    EXPECT_FALSE(v.setVersion(1, true));
    EXPECT_TRUE(v.setValue(1, V1));

    EXPECT_TRUE(v.setVersion(0, false));
    EXPECT_EQ(V0, v.value());

    EXPECT_FALSE(v.setVersion(1, true));
    EXPECT_EQ(V0, v.value());
    EXPECT_TRUE(v.setValue(1, V1));
    EXPECT_EQ(V1, v.value());

    EXPECT_FALSE(v.setVersion(2, true));
    EXPECT_TRUE(v.setValue(2, V2));

    EXPECT_TRUE(v.setVersion(1, false));
    EXPECT_EQ(V1, v.value());

    EXPECT_FALSE(v.setVersion(2, true));
    EXPECT_EQ(V1, v.value());
    EXPECT_TRUE(v.setValue(2, V2));
    EXPECT_EQ(V2, v.value());
    EXPECT_TRUE(v.setValue(2, V3));
    EXPECT_EQ(V3, v.value());
}

TEST(VersionedValueTest, VersionAccess)
{
    typedef VersionedValue<int> VVint;
    const int V0 = 8, V1 = 17, V3 = 22, V6 = 3;

    VVint v(V0);
    EXPECT_FALSE(v.setVersion(1, true));
    EXPECT_TRUE(v.setValue(1, V1));
    EXPECT_FALSE(v.setVersion(2, true));
    EXPECT_FALSE(v.setVersion(3, true));
    EXPECT_TRUE(v.setValue(3, V3));
    EXPECT_FALSE(v.setVersion(4, true));
    EXPECT_FALSE(v.setVersion(5, true));
    EXPECT_EQ(V3, v.value());
    EXPECT_FALSE(v.setVersion(6, true));
    EXPECT_TRUE(v.setValue(6, V6));
    EXPECT_EQ(V6, v.value());

    EXPECT_EQ(V0, v.value(0));
    EXPECT_EQ(V1, v.value(1));
    EXPECT_EQ(V1, v.value(2));
    EXPECT_EQ(V3, v.value(3));
    EXPECT_EQ(V3, v.value(4));
    EXPECT_EQ(V3, v.value(5));
    EXPECT_EQ(V6, v.value(6));

    EXPECT_TRUE(v.setVersion(2, true));
    EXPECT_EQ(V0, v.value(0));
    EXPECT_EQ(V1, v.value(1));
    EXPECT_EQ(V1, v.value(2));
    EXPECT_EQ(V1, v.value(3));
}

TEST(VersionedValueTest, HasVersion)
{
    typedef VersionedValue<int> VVint;
    const int V0 = 8, V1 = 17, V3 = 22, V6 = 3;

    VVint v(V0);
    EXPECT_FALSE(v.setVersion(1, true));
    EXPECT_TRUE(v.setValue(1, V1));
    EXPECT_FALSE(v.setVersion(2, true));
    EXPECT_FALSE(v.setVersion(3, true));
    EXPECT_TRUE(v.setValue(3, V3));
    EXPECT_FALSE(v.setVersion(4, true));
    EXPECT_FALSE(v.setVersion(5, true));
    EXPECT_EQ(V3, v.value());
    EXPECT_FALSE(v.setVersion(6, true));
    EXPECT_TRUE(v.setValue(6, V6));
    EXPECT_EQ(V6, v.value());

    EXPECT_TRUE (v.hasVersion(0));
    EXPECT_TRUE (v.hasVersion(1));
    EXPECT_FALSE(v.hasVersion(2));
    EXPECT_TRUE (v.hasVersion(3));
    EXPECT_FALSE(v.hasVersion(4));
    EXPECT_FALSE(v.hasVersion(5));
    EXPECT_TRUE (v.hasVersion(6));

    EXPECT_TRUE(v.setVersion(2, true));
    EXPECT_FALSE(v.hasVersion(2));
    EXPECT_FALSE(v.hasVersion(3));
}
