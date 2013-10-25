
#ifndef ParamIdModel_hh
#define ParamIdModel_hh 1

#include "util/VectorModel.hh"

struct ParamIdExtract : public VectorModelDetail::BasicExtract<int> {
  QVariant text(int paramId) const;
  QVariant tip(int paramId) const;
};
typedef VectorModel<ParamIdExtract> ParamIdModel;

typedef VectorModelDetail::OverrideExtract<ParamIdExtract> OParamIdExtract;
typedef VectorModel<OParamIdExtract> OverrideParamIdModel;


#endif // ParamIdModel_hh
