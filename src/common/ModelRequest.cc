
#include "ModelRequest.hh"

void ModelRequest::notifyData(const ModelData_pv& mdata)
{
  Q_EMIT data(mdata);
}

void ModelRequest::notifyStatus(const QString& withError)
{
  Q_EMIT completed(not withError.isNull());
}
