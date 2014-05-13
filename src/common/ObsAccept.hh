
#ifndef OBSACCEPT_HH
#define OBSACCEPT_HH 1

#include "ObsData.hh"
#include "ObsRequest.hh"

bool acceptST(ObsRequest_p request, const SensorTime& st);

inline bool acceptST(ObsRequest_p request, ObsData_p obs)
{ return obs and acceptST(request, obs->sensorTime()); }

bool acceptObs(ObsRequest_p request, ObsData_p obs);

#endif // OBSACCEPT_HH
