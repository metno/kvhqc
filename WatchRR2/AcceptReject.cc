
#include "AcceptReject.hh"

#include <kvalobs/kvDataOperations.h>

#define MILOGGER_CATEGORY "kvhqc.AcceptReject"
#include "HqcLogging.hh"

namespace AcceptReject {

int possibilities(EditDataPtr obs)
{
  // accumulated => disable
  // fmis=3 => disable
  // fmis=2 => disable accept for corrected, all others enabled
  // fmis=1 => disable reject, all others enabled
  
  METLIBS_LOG_SCOPE();
  
  const Sensor& s = obs->sensorTime().sensor;
  if (s.paramId == kvalobs::PARAMID_RR_24 or Helpers::is_accumulation(obs))
    // for accumulations, always use WatchRR
    return 0;
  
  const int fmis = obs->controlinfo().flag(kvalobs::flag::fmis);
  if (fmis == 3)
    return CAN_CORRECT;
  
  int possible = ALL;
  if (Helpers::is_rejected(obs))
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

void accept_original(EditAccessPtr eda, const SensorTime& sensorTime, bool qc2ok)
{
  EditDataPtr obs = eda->findE(sensorTime);
  if (not obs) {
    METLIBS_LOG_ERROR("accept_original without obs for " << sensorTime);
    return;
  }

  const kvalobs::kvControlInfo ci = obs->controlinfo();
  const int fmis = ci.flag(kvalobs::flag::fmis);
  if (fmis == 3) {
    METLIBS_LOG_ERROR("fmis=3, accept_original not possible for " << sensorTime);
    return;
  }
  if (not (fmis == 0 or fmis == 1 or fmis == 2 or fmis == 4)) {
    METLIBS_LOG_ERROR("bad accept_original, fmis != 0/1/2/4 for " << sensorTime);
    return;
  }

  EditDataEditorPtr editor = eda->editor(obs);
  editor->setCorrected(obs->original());

  Helpers::set_fhqc(editor, 1);
  if (fmis == 0 or fmis == 2) {
    Helpers::set_flag(editor, kvalobs::flag::fmis, 0);
    Helpers::set_flag(editor, kvalobs::flag::fd,   1);
  } else if (fmis == 1) {
    Helpers::set_flag(editor, kvalobs::flag::fmis, 3);
  } else if (fmis == 4) {
    Helpers::set_flag(editor, kvalobs::flag::fmis, 0);
  }
  if (qc2ok)
    Helpers::set_fhqc(editor, 4);

  editor->commit();
}

void accept_corrected(EditAccessPtr eda, const SensorTime& sensorTime, bool qc2ok)
{
  EditDataPtr obs = eda->findE(sensorTime);
  if (not obs) {
    METLIBS_LOG_ERROR("accept_corrected without obs for " << sensorTime);
    return;
  }

  const int fmis = obs->controlinfo().flag(kvalobs::flag::fmis);
  EditDataEditorPtr editor = eda->editor(obs);

  if (Helpers::float_eq()(obs->original(), editor->corrected())
      and (not Helpers::is_accumulation(editor)) and fmis < 2)
  {
    Helpers::set_flag(editor, kvalobs::flag::fd, 1);
    Helpers::set_fhqc(editor, 1);
  } else if (fmis == 0) {
    Helpers::set_fhqc(editor, 7);
  } else if (fmis == 1 or fmis == 4) {
    Helpers::set_fhqc(editor, 5);
  } else {
    METLIBS_LOG_ERROR("bad accept_corrected for " << sensorTime);
    return;
  }
  if (qc2ok)
    Helpers::set_fhqc(editor, 4);

  editor->commit();
}

void reject(EditAccessPtr eda, const SensorTime& sensorTime, bool qc2ok)
{
  EditDataPtr obs = eda->findE(sensorTime);
  if (not obs) {
    METLIBS_LOG_ERROR("reject without obs for " << sensorTime);
    return;
  }

  const int fmis = obs->controlinfo().flag(kvalobs::flag::fmis);
  if (fmis == 1 or fmis == 3) {
    METLIBS_LOG_ERROR("bad reject with fmis=1/3 for " << sensorTime);
    return;
  }

  EditDataEditorPtr editor = eda->editor(obs);
  Helpers::reject(editor);
  if (qc2ok)
    Helpers::set_fhqc(editor, 4);

  editor->commit();
}

} // namespace AcceptReject
