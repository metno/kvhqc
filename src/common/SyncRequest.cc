
#include "SyncRequest.hh"

#include "ModelAccess.hh"
#include "ModelRequest.hh"
#include "ObsAccess.hh"
#include "WrapRequest.hh"
#include "util/Synchronizer.hh"

#define MILOGGER_CATEGORY "kvhqc.SyncRequest"
#include "common/ObsLogging.hh"

#define USE_SYNCHRONIZER 1

namespace {

#ifndef USE_SYNCHRONIZER
class SyncedRequest : public WrapRequest
{
public:
  SyncedRequest(ObsRequest_p r)
      : WrapRequest(r)
  {
  }

  ~SyncedRequest();

  void run(ObsAccess_p access);

  void newData(const ObsData_pv& data) override { wrapped()->newData(data); }
  void updateData(const ObsData_pv& data) override { wrapped()->updateData(data); }
  void dropData(const SensorTime_v& dropped) override { wrapped()->dropData(dropped); }

  void completed(const QString& withError) override;

private:
  QSemaphore mSemaphore;
};

SyncedRequest::~SyncedRequest()
{
}

void SyncedRequest::run(ObsAccess_p access)
{
  METLIBS_LOG_SCOPE();
  access->postRequest(std::static_pointer_cast<ObsRequest>(shared_from_this()));
  METLIBS_LOG_DEBUG("acquire semaphore");
  Synchronizer::acquireWithSignalProcessing(mSemaphore);
}

void SyncedRequest::completed(const QString& withError)
{
  METLIBS_LOG_SCOPE();
  METLIBS_LOG_DEBUG("release semaphore");
  mSemaphore.release();
  METLIBS_LOG_DEBUG("calling wrap->completed");
  WrapRequest::completed(withError);
}
#endif // !USE_SYNCHRONIZER

} // anonymous namespace

ObsRequest_p syncRequest(ObsRequest_p request, ObsAccess_p access)
{
#ifdef USE_SYNCHRONIZER
  Synchronizer sync;
  QObject::connect(request.get(), SIGNAL(requestCompleted(const QString&)), &sync, SLOT(taskDone()));
  access->postRequest(request);
  sync.waitForSignal();
#else
  std::shared_ptr<SyncedRequest> sync = std::make_shared<SyncedRequest>(request);
  sync->run(access);
#endif
  return request;
}

//########################################################################

ModelRequest_p syncRequest(ModelRequest_p request, ModelAccess_p access)
{
  Synchronizer sync;
  QObject::connect(request.get(), SIGNAL(requestCompleted(const QString&)), &sync, SLOT(taskDone()));

  access->postRequest(request);
  sync.waitForSignal();
  
  QObject::disconnect(request.get(), SIGNAL(requestCompleted(const QString&)), &sync, SLOT(taskDone()));

  return request;
}
