
#include "SyncRequest.hh"
#include "SyncRequestPrivate.hh"

#include <QtCore/QCoreApplication>

#include <boost/make_shared.hpp>

#define MILOGGER_CATEGORY "kvhqc.SyncRequest"
#include "common/ObsLogging.hh"

// ========================================================================

ExecSyncRequest::ExecSyncRequest(ObsRequest_p request)
{
  mRequest = boost::dynamic_pointer_cast<SignalRequest>(request);
  if (not mRequest)
    mRequest = boost::make_shared<SignalRequest>(request);

  connect(mRequest.get(), SIGNAL(requestCompleted(bool)), this, SLOT(onCompleted(bool)));
}

// ------------------------------------------------------------------------

SignalRequest_p ExecSyncRequest::exec(ObsAccess_p access)
{
  METLIBS_LOG_SCOPE();

  access->postRequest(mRequest);

  QCoreApplication* app = 0;
  int timeout = 2;
  while (not semaphore.tryAcquire(1, timeout)) {
    if (not app) {
      app = QCoreApplication::instance();
      if (not app) {
        METLIBS_LOG_ERROR("no QCoreApplication instance, Qt signal delivery not working");
        mRequest->completed(true);
        break;
      }
    }
    app->processEvents(QEventLoop::ExcludeUserInputEvents);
    if (timeout < 128)
      timeout *= 2;
  }
  
  METLIBS_LOG_DEBUG(LOGVAL(timeout));
  return mRequest;
}

// ------------------------------------------------------------------------

void ExecSyncRequest::onCompleted(bool)
{
  METLIBS_LOG_SCOPE();
  semaphore.release();
}

// ========================================================================

ObsRequest_p syncRequest(ObsRequest_p request, ObsAccess_p access)
{
  ExecSyncRequest esr(request);
  return esr.exec(access);
}
