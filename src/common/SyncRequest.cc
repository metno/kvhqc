
#include "SyncRequest.hh"

#include "ModelAccess.hh"
#include "ModelRequest.hh"
#include "ObsAccess.hh"
#include "SignalRequest.hh"
#include "util/Synchronizer.hh"

#if 0
ObsRequest_p syncRequest(ObsRequest_p request, ObsAccess_p access)
{
  Synchronizer sync;
  QObject::connect(request.get(), SIGNAL(requestCompleted(const QString&)), &sync, SLOT(taskDone()));

  access->postRequest(request);
  sync.waitForSignal();
  
  QObject::disconnect(request.get(), SIGNAL(requestCompleted(const QString&)), &sync, SLOT(taskDone()));

  return request;
}
#endif

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
