
#include "KvServiceHelper.hh"

#include <gtest/gtest.h>

#define MILOGGER_CATEGORY "kvhqc.WatchRR2.gtestMain"
#include "util/HqcLogging.hh"

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  milogger::LoggingConfig log4cpp("-.!!=-:");

  KvServiceHelper kvsh;

  return RUN_ALL_TESTS();
}
