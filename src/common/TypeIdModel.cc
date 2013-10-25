
#include "TypeIdModel.hh"

#include "KvHelpers.hh"

QVariant  TypeIdExtract::tip(int typeId) const
{
  return Helpers::typeInfo(typeId);
}
