
#include "ModelRequest.hh"

void ModelRequest::notifyData(const ModelData_pv& mdata)
{
  Q_EMIT data(mdata);
}

void ModelRequest::notifyCompleted(bool failed)
{
  Q_EMIT completed(failed);
}
