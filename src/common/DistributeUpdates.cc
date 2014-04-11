
#include "DistributeUpdates.hh"

#define MILOGGER_CATEGORY "kvhqc.DistributeUpdates"
#include "common/ObsLogging.hh"

void DistributeUpdates::send()
{
  METLIBS_LOG_SCOPE();
  for (r_obs_t::const_iterator it=r_update.begin(); it != r_update.end(); ++it) {
    ObsRequest_p request = it->first;
    request->updateData(it->second);
  }

  for (r_obs_t::const_iterator it=r_insert.begin(); it != r_insert.end(); ++it) {
    ObsRequest_p request = it->first;
    request->newData(it->second);
  }

  for (r_st_t::const_iterator it=r_drop.begin(); it != r_drop.end(); ++it) {
    ObsRequest_p request = it->first;
    request->dropData(it->second);
  }
}
