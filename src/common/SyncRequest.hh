
#ifndef ACCESS_SYNCREQUEST_HH
#define ACCESS_SYNCREQUEST_HH 1

#include "util/boostutil.hh"

class ObsAccess;
class ObsRequest;

class ModelAccess;
class ModelRequest;

HQC_TYPEDEF_P(ObsAccess);
HQC_TYPEDEF_P(ObsRequest);

HQC_TYPEDEF_P(ModelAccess);
HQC_TYPEDEF_P(ModelRequest);

ObsRequest_p syncRequest(ObsRequest_p request, ObsAccess_p access);
ModelRequest_p syncRequest(ModelRequest_p request, ModelAccess_p access);

#endif // ACCESS_SYNCREQUEST_HH
