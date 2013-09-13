
#include <gtest/gtest.h>

#define MILOGGER_CATEGORY "kvhqc.WatchRR2.gtestMain"
#include "HqcLogging.hh"

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    milogger::LoggingConfig log4cpp("-.!!=-:");
    return RUN_ALL_TESTS();
}
