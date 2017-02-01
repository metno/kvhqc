
#include "AcceptReject.hh"

#include "KvHelpers.hh"
#include "ModelBuffer.hh"
#include "ObsHelpers.hh"
#include "Functors.hh"

#include <kvalobs/kvDataOperations.h>

#define MILOGGER_CATEGORY "kvhqc.AcceptReject"
#include "common/ObsLogging.hh"

namespace AcceptReject {

int possibilities(ObsData_p obs)
{
  // accumulated => disable
  // fmis=3 => disable
  // fmis=2 => disable accept for corrected, all others enabled
  // fmis=1 => disable reject, all others enabled
  
  METLIBS_LOG_SCOPE();
  if (not obs)
    return CAN_CORRECT;
  
  const Sensor& s = obs->sensorTime().sensor;
  if (s.paramId == kvalobs::PARAMID_RR_24 or Helpers::is_accumulation(obs))
    // for accumulations, always use WatchRR
    return 0;
  
  const kvalobs::kvControlInfo ci = obs->controlinfo();
  const int fmis = ci.flag(kvalobs::flag::fmis);
  if (fmis == 3)
    return CAN_CORRECT;
  
  int possible = ALL;
  if (ci.flag(kvalobs::flag::fhqc) == 0xA)
    possible &= ~CAN_REJECT;
  if (fmis == 3)
    possible &= ~CAN_ACCEPT_ORIGINAL;

  if (fmis == 2)
    possible &= ~CAN_ACCEPT_CORRECTED;
  if (fmis == 1)
    possible &= ~CAN_REJECT;

  return possible;
}

// ----------------------------------------

void accept_original(EditAccess_p ea, ObsData_p obs)
{
  const kvalobs::kvControlInfo& ci = obs->controlinfo();
  const int fmis = ci.flag(kvalobs::flag::fmis);
  if (fmis == 3) {
    HQC_LOG_ERROR("fmis=3, accept_original not possible for " << obs->sensorTime());
    return;
  }
  if (not (fmis == 0 or fmis == 1 or fmis == 2 or fmis == 4)) {
    HQC_LOG_ERROR("bad accept_original, fmis != 0/1/2/4 for " << obs->sensorTime());
    return;
  }

  ObsUpdate_p update = ea->createUpdate(obs);
  update->setCorrected(obs->original());

  Helpers::set_fhqc(update, 1);
  if (fmis == 2 or fmis == 4) {
    Helpers::set_flag(update, kvalobs::flag::fmis, 0);
  } else if (fmis == 1) {
    Helpers::set_flag(update, kvalobs::flag::fmis, 3);
  }

  ea->storeUpdates(ObsUpdate_pv(1, update));
}

void accept_model(EditAccess_p ea, ModelAccess_p mda, ObsData_p obs, bool qc2ok)
{
  ModelBuffer_p buffer = std::make_shared<ModelBuffer>(mda);
  ModelData_p md = buffer->getSync(obs->sensorTime());
  if (obs and md) {
    ObsUpdate_p update = ea->createUpdate(obs);
    update->setCorrected(md->value());
    ea->storeUpdates(ObsUpdate_pv(1, update));
    accept_corrected(ea, obs, qc2ok);
  }
}

void accept_corrected(EditAccess_p ea, ObsData_p obs, bool qc2ok)
{
  const int fmis = obs->controlinfo().flag(kvalobs::flag::fmis);
  ObsUpdate_p update = ea->createUpdate(obs);

  if (Helpers::float_eq()(obs->original(), update->corrected()) and fmis == 0) {
    Helpers::set_fhqc(update, 1);
  } else if (fmis == 0) {
    Helpers::set_fhqc(update, 7);
    Helpers::set_flag(update, kvalobs::flag::fmis, 4);
  } else if (fmis == 1 or fmis == 4) {
    Helpers::set_fhqc(update, 5);
  } else {
    HQC_LOG_ERROR("bad accept_corrected for " << obs->sensorTime());
    return;
  }
  if (qc2ok)
    Helpers::set_fhqc(update, 4);

  ea->storeUpdates(ObsUpdate_pv(1, update));
}

void reject(EditAccess_p ea, ObsData_p obs, bool qc2ok)
{
  const int fmis = obs->controlinfo().flag(kvalobs::flag::fmis);
  if (fmis == 1 or fmis == 3) {
    HQC_LOG_ERROR("bad reject with fmis=1/3 for " << obs->sensorTime());
    return;
  }

  ObsUpdate_p update = ea->createUpdate(obs);
  Helpers::reject(update, obs);
  if (qc2ok)
    Helpers::set_fhqc(update, 4);

  ea->storeUpdates(ObsUpdate_pv(1, update));
}

} // namespace AcceptReject
