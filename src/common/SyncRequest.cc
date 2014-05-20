
#include "SyncRequest.hh"

#include "ModelAccess.hh"
#include "ModelRequest.hh"
#include "ObsAccess.hh"
#include "SignalRequest.hh"
#include "util/Synchronizer.hh"

#include <boost/make_shared.hpp>

ObsRequest_p syncRequest(ObsRequest_p request, ObsAccess_p access)
{
  SignalRequest_p r = boost::dynamic_pointer_cast<SignalRequest>(request);
  if (not r)
    r = boost::make_shared<SignalRequest>(request);

  Synchronizer sync;
  QObject::connect(r.get(), SIGNAL(requestCompleted(bool)), &sync, SLOT(taskDone()));

  access->postRequest(r);
  sync.waitForSignal();
  
  QObject::disconnect(r.get(), SIGNAL(requestCompleted(bool)), &sync, SLOT(taskDone()));

  return r;
}

//########################################################################

ModelRequest_p syncRequest(ModelRequest_p request, ModelAccess_p access)
{
  Synchronizer sync;
  QObject::connect(request.get(), SIGNAL(completed(bool)), &sync, SLOT(taskDone()));

  access->postRequest(request);
  sync.waitForSignal();
  
  QObject::disconnect(request.get(), SIGNAL(completed(bool)), &sync, SLOT(taskDone()));

  return request;
}
