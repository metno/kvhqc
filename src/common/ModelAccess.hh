
#ifndef ModelAccess_hh
#define ModelAccess_hh 1

#include "ModelRequest.hh"

class ModelAccess : private boost::noncopyable {
public:
  enum { MODEL_TYPEID=99, MODEL_SENSOR=0 };

  virtual ~ModelAccess();

  virtual void postRequest(ModelRequest_p request) = 0;
  virtual void dropRequest(ModelRequest_p request) = 0;
};

HQC_TYPEDEF_P(ModelAccess);

#endif // ModelAccess_hh
