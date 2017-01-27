
#include "SimpleCorrectionsTest.hh"

#include "common/test/FakeKvApp.hh"

#include "common/KvMetaDataBuffer.hh"
#include "common/KvServiceHelper.hh"
#include <QApplication>

#include <QtTest/qtest.h>

#define MILOGGER_CATEGORY "kvhqc.common.qtestMain"
#include "util/HqcLogging.hh"

//#define DEBUG_MESSAGES

#ifdef DEBUG_MESSAGES
#include <log4cpp/Category.hh>
#endif // DEBUG_MESSAGES

#define EXEC_QTEST(TestObject)                                          \
  do {                                                                  \
    TestObject tc;                                                      \
    const int r = QTest::qExec(&tc, argc, argv);                        \
    if (r!= 0)                                                          \
      return r;                                                         \
  } while(false)

int main(int argc, char **argv)
{
  milogger::LoggingConfig log4cpp("-.!!=-:");
#ifdef DEBUG_MESSAGES
  log4cpp::Category::getRoot().setPriority(log4cpp::Priority::DEBUG);
#endif

  QApplication qapp(argc, argv);
  QDir::setSearchPaths("icons", QStringList(TOP_SOURCE_DIR "/share/images"));

  std::shared_ptr<FakeKvApp> fa(std::make_shared<FakeKvApp>(false)); // no threading
  KvServiceHelper kvsh(fa);
  KvMetaDataBuffer kvmdbuf;
  kvmdbuf.setHandler(fa->obsAccess()->handler());

  EXEC_QTEST(TestSimpleCorrections);

  return 0;
}
