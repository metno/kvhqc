
#ifndef common_TypeIdModel_hh
#define common_TypeIdModel_hh 1

#include "util/VectorModel.hh"

struct TypeIdExtract : public VectorModelDetail::BasicExtract<int> {
  QVariant text(int typeId) const
    { return QString::number(typeId); }
  QVariant tip(int typeId) const;
};
typedef VectorModel<TypeIdExtract> TypeIdModel;

typedef VectorModelDetail::OverrideExtract<TypeIdExtract> OTypeIdExtract;
typedef VectorModel<OTypeIdExtract> OverrideTypeIdModel;

#endif // common_TypeIdModel_hh
