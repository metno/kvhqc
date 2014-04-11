
#include "ObsAccept.hh"

bool acceptObs(ObsRequest_p request, ObsData_p obs)
{
  const SensorTime& st = obs->sensorTime();
  if (not request->timeSpan().contains(st.time))
    return false;

  if (not request->sensors().count(st.sensor))
    return false;

  ObsFilter_p filter = request->filter();
  if (filter and not filter->accept(obs, false))
    return false;

  return true;
}
