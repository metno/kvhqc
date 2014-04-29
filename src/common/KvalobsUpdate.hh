
#ifndef KVALOBSUPDATE_HH
#define KVALOBSUPDATE_HH 1

#include "KvalobsData.hh"
#include "ObsUpdate.hh"

class KvalobsUpdate : public ObsUpdate {
public:
  KvalobsUpdate(const SensorTime& st);

  KvalobsUpdate(ObsData_p obs);

  virtual const SensorTime& sensorTime() const
    { return mSensorTime; }
  
  virtual float corrected() const
    { return mCorrected; }

  virtual void setCorrected(float c)
    { mCorrected = c; }
  
  virtual const kvalobs::kvControlInfo& controlinfo() const
    { return mControlinfo; }

  virtual void setControlinfo(const kvalobs::kvControlInfo& ci)
    { mControlinfo = ci; }
  
  virtual const std::string& cfailed() const
    { return mCfailed; }
  
  virtual void setCfailed(const std::string& cf)
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
