
#include "ParamIdModel.hh"

#include "KvHelpers.hh"

QVariant ParamIdExtract::text(int paramId) const
{
  return KvMetaDataBuffer::instance()->paramName(paramId);
}

QVariant  ParamIdExtract::tip(int paramId) const
{
  return KvMetaDataBuffer::instance()->paramInfo(paramId);
}
