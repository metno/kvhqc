
#include "TypeIdModel.hh"

#include "KvHelpers.hh"

QVariant  TypeIdExtract::tip(int typeId) const
{
  return KvMetaDataBuffer::instance()->typeInfo(typeId);
}
