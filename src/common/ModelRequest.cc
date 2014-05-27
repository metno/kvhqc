
#include "ModelRequest.hh"

#include "QueryTask.hh"

void ModelRequest::notifyData(const ModelData_pv& mdata)
{
  Q_EMIT data(mdata);
}

void ModelRequest::notifyStatus(int status)
{
  if (status >= QueryTask::COMPLETE)
    Q_EMIT completed(status == QueryTask::FAILED);
}
