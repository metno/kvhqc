
#ifndef AcceptReject_hh
#define AcceptReject_hh 1

#include "common/ObsData.hh"
#include "common/EditAccess.hh"
#include "ModelAccess.hh"

namespace AcceptReject {

enum {
  CAN_REJECT           = 1<<0,
  CAN_ACCEPT_CORRECTED = 1<<1,
  CAN_ACCEPT_ORIGINAL  = 1<<2,
  CAN_CORRECT          = 1<<3,
  ALL = 0xF
};

int possibilities(ObsData_p obs);

void accept_model(EditAccess_p ea, ModelAccessPtr mda, ObsData_p obs, bool qc2ok);
void accept_corrected(EditAccess_p ea, ObsData_p obs, bool qc2ok);
void accept_original(EditAccess_p ea, ObsData_p obs);
void reject(EditAccess_p ea, ObsData_p obs, bool qc2ok);

} // namespace AcceptReject

#endif // AcceptReject_hh
