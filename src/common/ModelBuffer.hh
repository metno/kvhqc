
#ifndef MODELBUFFER_HH
#define MODELBUFFER_HH 1

#include "ModelAccess.hh"
#include "ModelData.hh"
#include "ModelRequest.hh"

class ModelBuffer : public QObject
{ Q_OBJECT;
public:
  ModelBuffer(ModelAccess_p ma);
  ~ModelBuffer();

  ModelData_p get(const SensorTime& st);
  ModelData_p getSync(const SensorTime& st);

  void clear();

Q_SIGNALS:
  void received(const ModelData_pv&);

private Q_SLOTS:
  void onRequestData(const ModelData_pv&);
  void onRequestCompleted(bool error);

private:
  void postRequest();
  void dropRequest();

  ModelRequest_p makeRequest(const SensorTime_v& sts);
  void dropRequest(ModelRequest_p request);

  ModelData_p cached(const SensorTime& st);

protected:
  ModelAccess_p mMA;
  ModelData_ps mCache;
  ModelRequest_p mRequest;
  SensorTime_v mPending;
};

HQC_TYPEDEF_P(ModelBuffer);

#endif // MODELBUFFER_HH