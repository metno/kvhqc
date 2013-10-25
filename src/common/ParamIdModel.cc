
#include "ParamIdModel.hh"

#include "KvHelpers.hh"

QVariant ParamIdExtract::text(int paramId) const
{
  return Helpers::paramName(paramId);
}

QVariant  ParamIdExtract::tip(int paramId) const
{
  return Helpers::paramInfo(paramId);
}
