
#include "KvHelpers.hh"
#include <kvalobs/kvDataOperations.h>
#include <gtest/gtest.h>

TEST(KvHelpersTest, getFlagText)
{
  ASSERT_EQ("", Helpers::getFlagText(std::string("0000000000000000")).toStdString());

  {
    kvalobs::kvControlInfo ci;
    ci.set(kvalobs::flag::ftime, 1);
    ASSERT_EQ(1, ci.flag(kvalobs::flag::ftime));
    ASSERT_EQ("ftime=1", Helpers::getFlagText(ci).toStdString());
  }

  {
    kvalobs::kvControlInfo ci;
    ci.set(kvalobs::flag::fd, 1);
    ASSERT_EQ(1, ci.flag(kvalobs::flag::fd));
    ASSERT_EQ("", Helpers::getFlagText(ci).toStdString());

    ci.set(kvalobs::flag::fd, 2);
    ASSERT_EQ(2, ci.flag(kvalobs::flag::fd));
    ASSERT_EQ("fd=2", Helpers::getFlagText(ci).toStdString());
  }
}

TEST(KvHelpersTest, formatValue)
{
  ASSERT_EQ("12.1", Helpers::formatValue(12.071f).toStdString());
  ASSERT_EQ("-1.1", Helpers::formatValue(-1.071f).toStdString());
}
