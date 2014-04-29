
#ifndef ACCESS_EDITACCESSPRIVATE_HH
#define ACCESS_EDITACCESSPRIVATE_HH 1

#include "EditAccess.hh"
#include "KvalobsData.hh"

#include "util/VersionedValue.hh"
#include "common/Functors.hh"

// ========================================================================

class EditUpdate : public ObsUpdate {
public:
  EditUpdate(const SensorTime& st)
    : mSensorTime(st) { }

  EditUpdate(ObsData_p obs)
    : mSensorTime(obs->sensorTime()), mObs(obs) { }

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

  SensorTime mSensorTime;
  ObsData_p mObs;

  float mCorrected;
  kvalobs::kvControlInfo mControlinfo;
  std::string mCfailed;
};

HQC_TYPEDEF_P(EditUpdate);
HQC_TYPEDEF_PV(EditUpdate);

// ========================================================================

class EditVersions {
public:
  EditVersions(ObsData_p backendData, size_t editVersion, ObsData_p editData);

  virtual const SensorTime& sensorTime() const
    { return currentData()->sensorTime(); }
  virtual float original() const
    { return currentData()->original(); }
  virtual float corrected() const
    { return currentData()->corrected(); }
  virtual const kvalobs::kvControlInfo& controlinfo() const
    { return currentData()->controlinfo(); }
  virtual const std::string& cfailed() const
    { return currentData()->cfailed(); }
  virtual const timeutil::ptime& tbtime() const
    { return currentData()->tbtime(); }

  void addVersion(size_t version, ObsData_p data);
  void setVersion(size_t version);
  void dropVersionsFrom(size_t version);

  ObsData_p versionData(size_t version) const;
  bool hasVersion(size_t version) const
    { return versionData(version); }

  void updateBackend(ObsData_p backendData);

  bool hasBackendData() const
    { return backendData(); }

  size_t currentVersion() const
    { return mVersions[mCurrent].version; }

  ObsData_p backendData() const
    { return mVersions.front().data; }

  ObsData_p currentData() const;

private:
  struct Version {
    size_t version;
    ObsData_p data;
    Version(size_t v, ObsData_p d) : version(v), data(d) { }
  };
  HQC_TYPEDEF_V(Version);

  Version_v mVersions;
  size_t mCurrent;
};

HQC_TYPEDEF_P(EditVersions);

struct lt_EditVersions : public std::binary_function<EditVersions_p, EditVersions_p, bool> {
  bool operator()(const EditVersions_p& a, const EditVersions_p& b) const
    { return lt_SensorTime()(a->sensorTime(), b->sensorTime()); }
  bool operator()(const SensorTime& a, const EditVersions_p& b) const
    { return lt_SensorTime()(a, b->sensorTime()); }
  bool operator()(const EditVersions_p& a, const SensorTime& b) const
    { return lt_SensorTime()(a->sensorTime(), b); }
};

typedef std::set<EditVersions_p, lt_EditVersions> EditVersions_ps;

// ========================================================================

class EditAccessPrivate;
typedef EditAccessPrivate* EditAccessPrivate_P;

class EditRequest;
HQC_TYPEDEF_P(EditRequest);

class EditRequest : public ObsRequest {
public:
  EditRequest(EditAccessPrivate_P a, ObsRequest_p wrapped);

  static EditRequest_p untag(ObsRequest_p wrapped);
  
  virtual const Sensor_s& sensors() const
    { return mWrapped->sensors(); }

  virtual const TimeSpan& timeSpan() const
    { return mWrapped->timeSpan(); }

  virtual ObsFilter_p filter() const
    { return mWrapped->filter(); }

  virtual void completed(bool failed);
  virtual void newData(const ObsData_pv& data);
  virtual void updateData(const ObsData_pv& data);
  virtual void dropData(const SensorTime_v& dropped);

  void use(EditVersions_p ev);
  void drop(EditVersions_p ev);

  EditAccessPrivate_P mAccess;
  ObsRequest_p mWrapped;
  EditVersions_ps mUsed;
};

HQC_TYPEDEF_X(EditRequest);
HQC_TYPEDEF_PV(EditRequest);
HQC_TYPEDEF_PS(EditRequest);

struct EditRequestUnwrap {
  ObsRequest_p operator()(EditRequest_p er) const
    { return er->mWrapped; }
};



// ========================================================================

class DistributeUpdates;

class EditAccessPrivate {
public:
  EditAccessPrivate(ObsAccess_p backend);

  EditVersions_ps::iterator findEditVersions(const SensorTime& st);
  void handleBackendNew(ObsRequest_p wr, const ObsData_pv& backendData);
  void handleBackendUpdate(ObsRequest_p wr, const ObsData_pv& backendData);
  void handleBackendDrop(ObsRequest_p wr, const SensorTime_v& backendDropped);

  void updateToPreviousVersion();
  void updateToNextVersion();

  ObsAccess_p mBackend;

  typedef std::vector<timeutil::ptime> VersionTimestamps_v;
  VersionTimestamps_v mVersionTimestamps;

  size_t mCurrentVersion;
  size_t mUpdated;

  EditVersions_ps mEdited;

  EditRequest_ps mRequests;
};

typedef EditAccess* EditAccess_P;
typedef EditAccessPrivate* EditAccessPrivate_P;

#endif // ACCESS_EDITACCESSPRIVATE_HH
