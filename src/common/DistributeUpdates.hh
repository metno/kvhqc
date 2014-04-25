
#ifndef DISTRIBUTEUPDATES_HH
#define DISTRIBUTEUPDATES_HH 1

#include "ObsData.hh"
#include "ObsRequest.hh"
#include <map>

class DistributeUpdates
{
public:
  typedef std::map<ObsRequest_p, ObsData_pv> r_obs_t;
  typedef std::map<ObsRequest_p, SensorTime_v> r_st_t;

  void updateData(ObsRequest_p request, ObsData_p obs)
    { r_update[request].push_back(obs); }

  void newData(ObsRequest_p request, ObsData_p obs)
    { r_insert[request].push_back(obs); }

  void dropData(ObsRequest_p request, ObsData_p obs)
    { dropData(request, obs->sensorTime()); }

  void dropData(ObsRequest_p request, const SensorTime& st)
    { r_drop[request].push_back(st); }

  void send();

  const r_obs_t& toUpdate() const
    { return r_update; }

  const r_obs_t& toInsert() const
    { return r_insert; }

  const r_st_t& toDrop() const
    { return r_drop; }

private:
  r_obs_t r_update, r_insert;
  r_st_t r_drop;

  ObsRequest_pv mRequests;
};

// ========================================================================

template<class R>
class DistributeRequestUpdates
{
public:
  DistributeRequestUpdates(const R& r)
    : mRequests(r) { }

  void updateData(ObsData_p obs);

  void newData(ObsData_p obs);

  void dropData(ObsData_p obs)
    { dropData(obs->sensorTime()); }

  void dropData(const SensorTime& st);

  void send()
    { mDU.send(); }

private:
  DistributeUpdates mDU;
  const R& mRequests;
};

template<class R>
void DistributeRequestUpdates<R>::updateData(ObsData_p obs)
{
  for (typename R::const_iterator itR = mRequests.begin(); itR != mRequests.end(); ++itR) {
    if (acceptObs(*itR, obs))
      mDU.updateData(*itR, obs);
  }
}

template<class R>
void DistributeRequestUpdates<R>::newData(ObsData_p obs)
{
  for (typename R::const_iterator itR = mRequests.begin(); itR != mRequests.end(); ++itR) {
    if (acceptObs(*itR, obs))
      mDU.newData(*itR, obs);
  }
}

template<class R>
void DistributeRequestUpdates<R>::dropData(const SensorTime& st)
{
  for (typename R::const_iterator itR = mRequests.begin(); itR != mRequests.end(); ++itR) {
    if ((*itR)->timeSpan().contains(st.time) and (*itR)->sensors().count(st.sensor))
      mDU.dropData(*itR, st);
  }
}

#endif // DISTRIBUTEUPDATES_HH
