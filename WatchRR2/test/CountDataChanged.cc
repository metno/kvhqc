
#include "CountDataChanged.hh"

#include "Helpers.hh"

CountDataChanged::CountDataChanged()
    : count(0)
    , filterWhat(ObsAccess::MODIFIED)
    , filterParam(kvalobs::PARAMID_RR_24)
{ }

void CountDataChanged::operator()(ObsAccess::ObsDataChange what, ObsDataPtr obs)
{
    if (what == filterWhat and obs->sensorTime().sensor.paramId == filterParam)
        count += 1;
}
