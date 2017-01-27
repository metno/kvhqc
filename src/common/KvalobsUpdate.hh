
#ifndef KVALOBSUPDATE_HH
#define KVALOBSUPDATE_HH 1

#include "KvalobsData.hh"
#include "ObsUpdate.hh"

class KvalobsUpdate : public ObsUpdate {
public:
  KvalobsUpdate(const SensorTime& st);

  KvalobsUpdate(ObsData_p obs);

  const SensorTime& sensorTime() const Q_DECL_OVERRIDE
    { return mSensorTime; }
  
  float corrected() const Q_DECL_OVERRIDE
    { return mCorrected; }

  void setCorrected(float c) Q_DECL_OVERRIDE
    { mCorrected = c; }
  
  const kvalobs::kvControlInfo& controlinfo() const Q_DECL_OVERRIDE
    { return mControlinfo; }

  void setControlinfo(const kvalobs::kvControlInfo& ci) Q_DECL_OVERRIDE
    { mControlinfo = ci; }
  
  const std::string& cfailed() const Q_DECL_OVERRIDE
    { return mCfailed; }
  
  void setCfailed(const std::string& cf) Q_DECL_OVERRIDE
    { mCfailed = cf; }

  ObsData_p obs()
    { return mObs; }

private:
  SensorTime mSensorTime;
  ObsData_p mObs;

  float mCorrected;
  kvalobs::kvControlInfo mControlinfo;
  std::string mCfailed;
};

HQC_TYPEDEF_P(KvalobsUpdate);
HQC_TYPEDEF_PV(KvalobsUpdate);

#endif // KVALOBSUPDATE_HH
