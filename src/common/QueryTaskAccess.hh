
#ifndef ACCESS_QUERYTASKACCESS_HH
#define ACCESS_QUERYTASKACCESS_HH 1

#include "ObsAccess.hh"
#include "QueryTaskHandler.hh"

class QueryTaskAccess : public QObject, public ObsAccess
{ Q_OBJECT;
public:
  QueryTaskAccess(QueryTaskHandler_p handler);
  ~QueryTaskAccess();

  virtual void postRequest(ObsRequest_p request);
  virtual void dropRequest(ObsRequest_p request);

protected:
  const ObsRequest_pv& requests() const
    { return mRequests; }

  void distributeUpdates(const ObsData_pv& updated, const ObsData_pv& inserted, const SensorTime_v& dropped);
  QueryTask* taskForRequest(ObsRequest_p request);

  QueryTaskHandler_p handler() const
    { return mHandler; }

private Q_SLOTS:
  void onNewData(ObsRequest_p request, const ObsData_pv& data);
  void onStatus(ObsRequest_p request, int);

private:
  bool isKnownRequest(ObsRequest_p request) const;

private:
  QueryTaskHandler_p mHandler;
  ObsRequest_pv mRequests;
};

HQC_TYPEDEF_P(QueryTaskAccess);

#endif // ACCESS_QUERYTASKACCESS_HH
