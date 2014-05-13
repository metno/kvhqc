
#ifndef DISTRIBUTEUPDATES_HH
#define DISTRIBUTEUPDATES_HH 1

#include "ObsAccept.hh"
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

struct DistributeRequestUnwrap {
  ObsRequest_p operator()(ObsRequest_p r) const
    { return r; }
};

template<class R, class U = DistributeRequestUnwrap>
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

template<class R, class U>
void DistributeRequestUpdates<R, U>::updateData(ObsData_p obs)
{
  const U unwrap;
  for (typename R::const_iterator itR = mRequests.begin(); itR != mRequests.end(); ++itR) {
    ObsRequest_p r = unwrap(*itR);
    if (acceptST(r, obs))
      mDU.updateData(r, obs);
  }
}

template<class R, class U>
void DistributeRequestUpdates<R, U>::newData(ObsData_p obs)
{
  const U unwrap;
  for (typename R::const_iterator itR = mRequests.begin(); itR != mRequests.end(); ++itR) {
    ObsRequest_p r = unwrap(*itR);
    if (acceptObs(r, obs))
      mDU.newData(r, obs);
  }
}

template<class R, class U>
void DistributeRequestUpdates<R, U>::dropData(const SensorTime& st)
{
  const U unwrap;
  for (typename R::const_iterator itR = mRequests.begin(); itR != mRequests.end(); ++itR) {
    ObsRequest_p r = unwrap(*itR);
    if (acceptST(r, st))
      mDU.dropData(r, st);
  }
}

#endif // DISTRIBUTEUPDATES_HH
