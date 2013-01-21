
#include "Helpers.hh"
#include <kvalobs/kvDataOperations.h>
#include <gtest/gtest.h>

TEST(HelpersTest, getFlagText)
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

TEST(HelpersTest, formatValue)
{
    ASSERT_EQ("12.1", kvalobs::formatValue(12.071f).toStdString());
    ASSERT_EQ("-1.1", kvalobs::formatValue(-1.071f).toStdString());
}

TEST(HelpersTest, roundDecimals)
{
    ASSERT_FLOAT_EQ(12.1f, Helpers::roundDecimals(12.1f, 1));
    ASSERT_FLOAT_EQ(12.0f, Helpers::roundDecimals(12.1f, 0));
    ASSERT_FLOAT_EQ(10.0f, Helpers::roundDecimals(12.1f, -1));

    ASSERT_FLOAT_EQ(-3.6f, Helpers::roundDecimals(-3.6f, 1));
    ASSERT_FLOAT_EQ(-4.0f, Helpers::roundDecimals(-3.6f, 0));
    ASSERT_FLOAT_EQ( 0.0f, Helpers::roundDecimals(-3.6f, -1));
}

TEST(HelpersTest, parseFloat)
{
    ASSERT_THROW(Helpers::parseFloat("12.1", 0), std::runtime_error);
    try {
        Helpers::parseFloat("12.1", 1);
    } catch(std::runtime_error& e) {
        FAIL() << e.what();
    }
    ASSERT_NO_THROW(Helpers::parseFloat("12", 0));

    ASSERT_THROW(Helpers::parseFloat("-3.1", 0), std::runtime_error);
    ASSERT_NO_THROW(Helpers::parseFloat("-3.1", 1));
    ASSERT_NO_THROW(Helpers::parseFloat("-3", 0));
}

TEST(HelpersTest, appendedText)
{
    ASSERT_EQ("wo, ho", Helpers::appendedText("wo", "ho", ", ").toStdString());
    ASSERT_EQ("wo", Helpers::appendedText("wo", "", ", ").toStdString());
    ASSERT_EQ("ho", Helpers::appendedText("", "ho", ", ").toStdString());
    ASSERT_EQ("", Helpers::appendedText("", "", ", ").toStdString());
}
