
#ifndef KVALOBSUPDATE_HH
#define KVALOBSUPDATE_HH 1

#include "KvalobsData.hh"
#include "ObsUpdate.hh"

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
  
  enum { CHANGED_CORRECTED = 1, CHANGED_CONTROLINFO = 2, CHANGED_CFAILED = 4, CHANGED_NEW = 8 };

  int changes() const
    { return mChanged; }

  const kvalobs::kvData& data() const
    { return mData; }

private:
  SensorTime mSensorTime;
  kvalobs::kvData mData;
  int mChanged;
  float mNewCorrected;
  kvalobs::kvControlInfo mNewControlinfo;
  std::string mNewCfailed;
};

HQC_TYPEDEF_P(KvalobsUpdate);
HQC_TYPEDEF_PV(KvalobsUpdate);

#endif // KVALOBSUPDATE_HH
