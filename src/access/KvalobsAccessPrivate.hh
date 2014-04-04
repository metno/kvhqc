
#ifndef ACCESS_KVALOBSACCESSPRIVATE_HH
#define ACCESS_KVALOBSACCESSPRIVATE_HH 1

#include "KvalobsAccess.hh"
#include "KvalobsData.hh"

#include <QtSql/QSqlDatabase>

// ========================================================================

class KvalobsHandler : public BackgroundHandler
{
public:
  virtual void initialize();
  virtual void finalize();

  virtual ObsData_pv queryData(ObsRequest_p request);

private:
  QSqlDatabase mKvalobsDB;
};

// ========================================================================

class KvalobsUpdate : public ObsUpdate {
public:
  KvalobsUpdate(KvalobsData_p kvdata);
  KvalobsUpdate(const SensorTime& st);

  virtual const SensorTime& sensorTime() const
    { return mSensorTime; }
  
  virtual float corrected() const
    { return mNewCorrected; }

  virtual void setCorrected(float c);
  
  virtual const kvalobs::kvControlInfo& controlinfo() const
    { return mNewControlinfo; }

  virtual void setControlinfo(const kvalobs::kvControlInfo& ci);
  
  virtual const std::string& cfailed() const
    { return mNewCfailed; }
  
  virtual void setCfailed(const std::string& cf);
  
  int changes() const
    { return mChanged; }

private:
  enum { CHANGED_CORRECTED = 1, CHANGED_CONTROLINFO = 2, CHANGED_CFAILED = 4, CHANGED_NEW = 8 };

  SensorTime mSensorTime;
  kvalobs::kvData mData;
  int mChanged;
  float mNewCorrected;
  kvalobs::kvControlInfo mNewControlinfo;
  std::string mNewCfailed;
};

HQC_TYPEDEF_P(KvalobsUpdate);
HQC_TYPEDEF_PV(KvalobsUpdate);

#endif // ACCESS_KVALOBSACCESSPRIVATE_HH
