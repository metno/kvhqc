
#ifndef ACCESS_EDITACCESSPRIVATE_HH
#define ACCESS_EDITACCESSPRIVATE_HH 1

#include "EditAccess.hh"

// ========================================================================

class EditUpdate : public ObsUpdate {
public:
  EditUpdate(ObsData_p backendData);

  virtual SensorTime sensorTime() const
    { return mBackendData->sensorTime(); }
  
  virtual void setCorrected(float c);
  
  virtual void setControlinfo(const kvalobs::kvControlInfo& ci);
  
  virtual void setCfailed(const std::string& cf);

  float corrected() const
    { return mNewCorrected; }
  
  const kvalobs::kvControlInfo& controlinfo() const
    { return mNewControlinfo; }
  
  const std::string& cfailed() const
    { return mNewCfailed; }

private:  
  float mNewCorrected;
  kvalobs::kvControlInfo mNewControlinfo;
  std::string mNewCfailed;
};

// ========================================================================

class EditVersions : public ObsData {
public:
  EditVersions(ObsData_p backendData);

  virtual const SensorTime& sensorTime() const
    { return mBackendData->sensorTime(); }
  virtual float original() const
    { return mBackendData->original(); }
  virtual float corrected() const
    { return mCorrected.value(); }
  virtual const kvalobs::kvControlInfo& controlinfo() const
    { return mControlinfo.value(); }
  virtual const std::string& cfailed() const
    { return mCfailed.value(); }
  virtual const timeutil::ptime& tbtime() const
    { return mBackendData->tbtime(); }
  
  virtual void set(int version, float corrected, const edit::kvControlInfo& controlinfo, const std::string& cfailed);
  
  bool modified() const
    { return mCorrected.modified() or mControlinfo.modified() or mCfailed.modified(); }

  ObsData_p mBackendData;

  typedef VersionedValue<float, Helpers::float_eq> VersionedCorrected_t;
  typedef VersionedValue<kvalobs::kvControlInfo>   VersionedControlinfo_t;
  typedef VersionedValue<std::string>              VersionedCfailed_t;

  VersionedCorrected_t   mCorrected;
  VersionedControlinfo_t mControlinfo;
  VersionedCfailed_t     mCfailed;

};

HQC_TYPEDEF_P(EditUpdate);
HQC_TYPEDEF_PV(EditUpdate);

#endif // ACCESS_EDITACCESSPRIVATE_HH
