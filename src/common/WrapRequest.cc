
#include "WrapRequest.hh"

WrapRequest::WrapRequest(ObsRequest_p wrapped)
  : mWrapped(wrapped)
{
  mWrapped->setTag(this);
}

WrapRequest::~WrapRequest()
{
  mWrapped->setTag(0);
}

WrapRequest_p WrapRequest::untag(ObsRequest_p wrapped)
{
  WrapRequest_x er = static_cast<WrapRequest_x>(wrapped->tag());
  return boost::static_pointer_cast<WrapRequest>(er->shared_from_this());
}

void WrapRequest::completed(const QString& withError)
{
  mWrapped->completed(withError);
  ObsRequest::completed(withError);
}
