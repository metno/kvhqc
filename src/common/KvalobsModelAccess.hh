
#ifndef KvalobsModelAccess_hh
#define KvalobsModelAccess_hh 1

#include "ModelAccess.hh"
#include "ModelData.hh"
#include "QueryTaskHandler.hh"

class ModelRequest;
HQC_TYPEDEF_P(ModelRequest);
HQC_TYPEDEF_PV(ModelRequest);

class KvalobsModelAccess : public QObject, public ModelAccess
{ Q_OBJECT;
public:
  KvalobsModelAccess(QueryTaskHandler_p handler);
  ~KvalobsModelAccess();

  void postRequest(ModelRequest_p request);
  void dropRequest(ModelRequest_p request);

  void cleanCache();

private Q_SLOTS:
  void modelData(const ModelData_pv&);

private:
  QueryTaskHandler_p mHandler;
  ModelRequest_pv mRequests;

  typedef std::map<SensorTime, ModelData_p, lt_ModelSensorTime> ModelDataCache_t;
  ModelDataCache_t mCache;
};

#endif // KvalobsModelAccess_hh
