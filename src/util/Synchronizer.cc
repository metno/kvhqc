
#include "Synchronizer.hh"

#include <QCoreApplication>

#define MILOGGER_CATEGORY "kvhqc.Synchronizer"
#include "HqcLogging.hh"

bool Synchronizer::waitForSignal()
{
  bool error = false;
  
  QCoreApplication* app = 0;
  int timeout = 2;
  while (not semaphore.tryAcquire(1, timeout)) {
    if (not app) {
      app = QCoreApplication::instance();
      if (not app) {
        HQC_LOG_ERROR("no QCoreApplication instance, Qt signal delivery not working");
        error = true;
        break;
      }
    }
    app->processEvents(QEventLoop::ExcludeUserInputEvents);
    if (timeout < 128)
      timeout *= 2;
  }
  
  return error;
}

// ------------------------------------------------------------------------

void Synchronizer::taskDone()
{
  semaphore.release();
}
