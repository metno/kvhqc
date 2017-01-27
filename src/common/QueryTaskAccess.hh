
#ifndef ACCESS_QUERYTASKACCESS_HH
#define ACCESS_QUERYTASKACCESS_HH 1

#include "ObsAccess.hh"
#include "QueryTaskHandler.hh"

class QueryTaskHelper;

class QueryTaskAccess : public QObject, public ObsAccess
{ Q_OBJECT;
public:
  QueryTaskAccess(QueryTaskHandler_p handler);
  ~QueryTaskAccess();

  void postRequest(ObsRequest_p request, bool synchronized=false) Q_DECL_OVERRIDE;
  void dropRequest(ObsRequest_p request) Q_DECL_OVERRIDE;

protected:
  const ObsRequest_pv& requests() const
    { return mRequests; }

  void distributeUpdates(const ObsData_pv& updated, const ObsData_pv& inserted, const SensorTime_v& dropped);

  QueryTaskHandler_p handler() const
    { return mHandler; }

private Q_SLOTS:
  void onNewData(ObsRequest_p request, const ObsData_pv& data);
  void onDone(ObsRequest_p request, const QString& withError);

private:
  bool isKnownRequest(ObsRequest_p request) const;
  QueryTaskHelper* taskForRequest(ObsRequest_p request);

private:
  QueryTaskHandler_p mHandler;
  ObsRequest_pv mRequests;
};

HQC_TYPEDEF_P(QueryTaskAccess);

#endif // ACCESS_QUERYTASKACCESS_HH
