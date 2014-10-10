
#include "ModelRequest.hh"

void ModelRequest::notifyData(const ModelData_pv& mdata)
{
  Q_EMIT data(mdata);
}

void ModelRequest::notifyDone(const QString& withError)
{
  Q_EMIT requestCompleted(withError);
}
