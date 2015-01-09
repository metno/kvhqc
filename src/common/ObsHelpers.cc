
#include "ObsHelpers.hh"

#include "KvHelpers.hh"
#include "FlagChange.hh"
#include "ModelAccess.hh"

#include <kvalobs/kvDataOperations.h>
#include <kvalobs/kvModelData.h>

namespace Helpers {

// ------------------------------------------------------------------------

int is_accumulation(ObsData_p obs)
{ return is_accumulation(obs->controlinfo()); }

int is_accumulation(ObsUpdate_p update)
{ return is_accumulation(update->controlinfo()); }

// ------------------------------------------------------------------------

int is_endpoint(ObsData_p obs)
{ return is_endpoint(obs->controlinfo()); }

int is_endpoint(ObsUpdate_p update)
{ return is_endpoint(update->controlinfo()); }

// ------------------------------------------------------------------------

bool is_rejected(ObsData_p obs)
{ return is_rejected(obs->controlinfo(), obs->corrected()); }

bool is_rejected(ObsUpdate_p update)
{ return is_rejected(update->controlinfo(), update->corrected()); }

// ------------------------------------------------------------------------

bool is_missing(ObsData_p obs)
{ return is_missing(obs->controlinfo(), obs->corrected()); }

bool is_missing(ObsUpdate_p update)
{ return is_missing(update->controlinfo(), update->corrected()); }

// ------------------------------------------------------------------------

bool is_orig_missing(ObsData_p obs)
{ return is_missing(obs->controlinfo(), obs->original()); }

bool is_orig_missing(ObsUpdate_p update, ObsData_p obs)
{ return not obs or is_orig_missing(update->controlinfo(), obs->original()); }

// ------------------------------------------------------------------------

bool is_valid(ObsData_p obs) // same as kvDataOperations.cc
{ return not is_missing(obs) and not is_rejected(obs); }

bool is_valid(ObsUpdate_p update) // same as kvDataOperations.cc
{ return not is_missing(update) and not is_rejected(update); }

// ------------------------------------------------------------------------

void reject(ObsUpdate_p update, ObsData_p obs) // same as kvDataOperations.cc
{
#if 0
  if (not is_valid(update))
    return;
#else
  // this is different from kvDataOperations.cc: allow rejecting a
  // rejected value, in effect setting fhqc=A such that the rejected
  // observation does not appear in the error list any more
  if (is_missing(update))
    return;
#endif
  
  const FlagChange fc_reject("fmis=[04]->fmis=2;fmis=1->fmis=3;fhqc=A");
  Helpers::changeControlinfo(update, fc_reject);
  if (is_orig_missing(update, obs))
    update->setCorrected(kvalobs::MISSING);
  else
    update->setCorrected(kvalobs::REJECTED);
}

void correct(ObsUpdate_p update, float newC)
{
  const FlagChange fc_diff("fmis=3->fmis=1;fmis=[02]->fmis=4;fhqc=7");
  Helpers::changeControlinfo(update, fc_diff);
  update->setCorrected(newC);
}

void set_flag(ObsUpdate_p update, int flag, int value)
{
  kvalobs::kvControlInfo ci = update->controlinfo();
  ci.set(flag, value);
  update->setControlinfo(ci);
}

void set_fhqc(ObsUpdate_p update, int fhqc)
{
  set_flag(update, kvalobs::flag::fhqc, fhqc);
}

void auto_correct(ObsUpdate_p update, ObsData_p obs, float newC)
{
  const bool interpolation = is_orig_missing(update, obs);
  correct(update, newC);
  if (interpolation)
    set_fhqc(update, 5);
}

int kvSensorNumber(const kvalobs::kvData& d)
{
  const int s = d.sensor();
  return (s>='0') ? (s-'0') : s;
}

Sensor sensorFromKvData(const kvalobs::kvData& d)
{
  return Sensor(d.stationID(), d.paramID(), d.level(), kvSensorNumber(d), d.typeID());
}

SensorTime sensorTimeFromKvData(const kvalobs::kvData& d)
{
  return SensorTime(sensorFromKvData(d), timeutil::from_miTime(d.obstime()));
}

Sensor sensorFromKvModelData(const kvalobs::kvModelData& d)
{
  return Sensor(d.stationID(), d.paramID(), d.level(), ModelAccess::MODEL_SENSOR, ModelAccess::MODEL_TYPEID);
}

SensorTime sensorTimeFromKvModelData(const kvalobs::kvModelData& d)
{
  return SensorTime(sensorFromKvModelData(d), timeutil::from_miTime(d.obstime()));
}

// ------------------------------------------------------------------------

int extract_ui2(ObsData_p obs)
{
  kvalobs::kvUseInfo ui;
  ui.setUseFlags(obs->controlinfo());
  return ui.flag(2);
}

} // namespace Helpers
