
#include "ObsAccept.hh"

bool acceptST(ObsRequest_p request, const SensorTime& st)
{
  if (not request->timeSpan().contains(st.time))
    return false;

  // FIXME this is a hack
  const Sensor_s& rsensors = request->sensors();
  if (rsensors.size() == 1 and not rsensors.begin()->valid())
    return true;
  if (not request->sensors().count(st.sensor))
    return false;

  return true;
}

bool acceptObs(ObsRequest_p request, ObsData_p obs)
{
  if (not acceptST(request, obs))
    return false;

  ObsFilter_p filter = request->filter();
  if (filter and not filter->accept(obs, false))
    return false;

  return true;
}
