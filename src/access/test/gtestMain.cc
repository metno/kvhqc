
#include "common/KvServiceHelper.hh"
#include <QtCore/QCoreApplication>

#include <gtest/gtest.h>

#define MILOGGER_CATEGORY "kvhqc.access.gtestMain"
#include "util/HqcLogging.hh"

//#define DEBUG_MESSAGES

#ifdef DEBUG_MESSAGES
#include <log4cpp/Category.hh>
#endif // DEBUG_MESSAGES

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  milogger::LoggingConfig log4cpp("-.!!=-:");
#ifdef DEBUG_MESSAGES
  log4cpp::Category::getRoot().setPriority(log4cpp::Priority::DEBUG);
#endif

  KvServiceHelper kvsh;
  QCoreApplication app(argc, argv);

  return RUN_ALL_TESTS();
}
