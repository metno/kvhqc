
#include "FakeModelAccess.hh"

#include "util/timeutil.hh"

#include <kvalobs/kvModelData.h>

void FakeModelAccess::insert(int stationId, int paramId, const std::string& obstime, float value)
{
  const kvalobs::kvModelData model(stationId, timeutil::to_miTime(timeutil::from_iso_extended_string(obstime)),
      paramId, 0, 0, value);
  receive(model);
}

bool FakeModelAccess::erase(ModelDataPtr mdl)
{
  return drop(mdl->sensorTime());
}
