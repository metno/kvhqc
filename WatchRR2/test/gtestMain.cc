
#include <gtest/gtest.h>

#define MILOGGER_CATEGORY "kvhqc.WatchRR2.gtestMain"
#include "HqcLogging.hh"
#include <log4cpp/Category.hh>

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  milogger::LoggingConfig log4cpp("-.!!=-:");
  //log4cpp::Category::getRoot().setPriority(log4cpp::Priority::DEBUG);
  return RUN_ALL_TESTS();
}
