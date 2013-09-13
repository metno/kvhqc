
#include "ObsHelpers.hh"

#include "FlagChange.hh"
#include "ModelAccess.hh"

#include <kvalobs/kvDataOperations.h>
#include <kvalobs/kvModelData.h>

namespace Helpers {

// ------------------------------------------------------------------------

int is_accumulation(ObsDataPtr obs)
{ return is_accumulation(obs->controlinfo()); }

int is_accumulation(EditDataEditorPtr editor)
{ return is_accumulation(editor->controlinfo()); }

// ------------------------------------------------------------------------

int is_endpoint(ObsDataPtr obs)
{ return is_endpoint(obs->controlinfo()); }

int is_endpoint(EditDataEditorPtr editor)
{ return is_endpoint(editor->controlinfo()); }

// ------------------------------------------------------------------------

bool is_rejected(ObsDataPtr obs)
{ return is_rejected(obs->controlinfo(), obs->corrected()); }

bool is_rejected(EditDataEditorPtr editor)
{ return is_rejected(editor->controlinfo(), editor->corrected()); }

// ------------------------------------------------------------------------

bool is_missing(ObsDataPtr obs)
{ return is_missing(obs->controlinfo(), obs->corrected()); }

bool is_missing(EditDataEditorPtr editor)
{ return is_missing(editor->controlinfo(), editor->corrected()); }

// ------------------------------------------------------------------------

bool is_orig_missing(ObsDataPtr obs)
{ return is_missing(obs->controlinfo(), obs->original()); }

bool is_orig_missing(EditDataEditorPtr editor)
{ return is_orig_missing(editor->controlinfo(), editor->obs()->original()); }

// ------------------------------------------------------------------------

bool is_valid(ObsDataPtr obs) // same as kvDataOperations.cc
{ return not is_missing(obs) and not is_rejected(obs); }

bool is_valid(EditDataEditorPtr editor) // same as kvDataOperations.cc
{ return not is_missing(editor) and not is_rejected(editor); }

// ------------------------------------------------------------------------

void reject(EditDataEditorPtr editor) // same as kvDataOperations.cc
{
    if (not is_valid(editor))
        return;
    
    const FlagChange fc_reject("fmis=[04]->fmis=2;fmis=1->fmis=3;fhqc=A");
    editor->changeControlinfo(fc_reject);
    if (is_orig_missing(editor))
        editor->setCorrected(kvalobs::MISSING);
    else
        editor->setCorrected(kvalobs::REJECTED);
}

void correct(EditDataEditorPtr editor, float newC)
{
    const FlagChange fc_diff("fmis=3->fmis=1;fmis=[02]->fmis=4;fhqc=7");
    editor->changeControlinfo(fc_diff);
    editor->setCorrected(newC);
}

void set_flag(EditDataEditorPtr editor, int flag, int value)
{
    kvalobs::kvControlInfo ci = editor->controlinfo();
    ci.set(flag, value);
    editor->setControlinfo(ci);
}

void set_fhqc(EditDataEditorPtr editor, int fhqc)
{
  set_flag(editor, kvalobs::flag::fhqc, fhqc);

  const int fmis = editor->controlinfo().flag(kvalobs::flag::fmis);
  if (fmis == 0)
    set_flag(editor, kvalobs::flag::fmis, 4);
}

void auto_correct(EditDataEditorPtr editor, float newC)
{
    const bool interpolation = is_orig_missing(editor);
    correct(editor, newC);
    if (interpolation)
        set_fhqc(editor, 5);
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

int extract_ui2(ObsDataPtr obs)
{
  kvalobs::kvUseInfo ui;
  ui.setUseFlags(obs->controlinfo());
  return ui.flag(2);
}

} // namespace Helpers
