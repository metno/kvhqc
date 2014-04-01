
#ifndef AcceptReject_hh
#define AcceptReject_hh 1

#include "access/EditAccess.hh"
#include "ModelAccess.hh"

namespace AcceptReject {

enum {
  CAN_REJECT           = 1<<0,
  CAN_ACCEPT_CORRECTED = 1<<1,
  CAN_ACCEPT_ORIGINAL  = 1<<2,
  CAN_CORRECT          = 1<<3,
  ALL = 0xF
};

int possibilities(EditDataPtr obs);

void accept_model(EditAccessPtr eda, ModelAccessPtr mda, const SensorTime& sensorTime, bool qc2ok);
void accept_corrected(EditAccessPtr eda, const SensorTime& sensorTime, bool qc2ok);
void accept_original(EditAccessPtr eda, const SensorTime& sensorTime);
void reject(EditAccessPtr eda, const SensorTime& sensorTime, bool qc2ok);

} // namespace AcceptReject

#endif // AcceptReject_hh
