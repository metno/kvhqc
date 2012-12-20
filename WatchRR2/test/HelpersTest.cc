
#include "Helpers.hh"
#include <kvalobs/kvDataOperations.h>
#include <gtest/gtest.h>

TEST(HelpersTest, getFlagText)
{
    ASSERT_EQ(QString(""), Helpers::getFlagText(std::string("0000000000000000")));

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

TEST(HelpersTest, formatValue)
{
    ASSERT_EQ(QString("12.1"), kvalobs::formatValue(12.071f));
    ASSERT_EQ(QString("-1.1"), kvalobs::formatValue(-1.071f));
}


TEST(HelpersTest, appendedText)
{
    ASSERT_EQ(QString("wo, ho"), Helpers::appendedText("wo", "ho", ", "));
    ASSERT_EQ(QString("wo"), Helpers::appendedText("wo", "", ", "));
    ASSERT_EQ(QString("ho"), Helpers::appendedText("", "ho", ", "));
    ASSERT_EQ(QString(""), Helpers::appendedText("", "", ", "));
}
