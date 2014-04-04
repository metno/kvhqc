
#ifndef ACCESS_EDITACCESSPRIVATE_HH
#define ACCESS_EDITACCESSPRIVATE_HH 1

#include "EditAccess.hh"

#include "util/VersionedValue.hh"
#include "common/Functors.hh"

// ========================================================================

class EditUpdate : public ObsUpdate {
public:
  EditUpdate(const SensorTime& st, bool created)
    : mSensorTime(st), mCreated(created) { }

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

  SensorTime mSensorTime;
  bool mCreated;

  float mCorrected;
  kvalobs::kvControlInfo mControlinfo;
  std::string mCfailed;
};

HQC_TYPEDEF_P(EditUpdate);
HQC_TYPEDEF_PV(EditUpdate);

// ========================================================================

class EditVersions : public ObsData {
public:
  EditVersions(ObsData_p backendData);
  EditVersions(const SensorTime& sensorTime);

  virtual const SensorTime& sensorTime() const
    { return mSensorTime; }
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
  
  bool set(size_t version, bool drop);
  bool hasVersion(size_t version) const;

  bool set(size_t version, float corrected, const kvalobs::kvControlInfo& controlinfo, const std::string& cfailed);
  bool modified() const
    { return mCorrected.modified() or mControlinfo.modified() or mCfailed.modified(); }
  bool updateBackend(ObsData_p backendData);

  void use()
    { mUsed += 1; }
  bool drop()
    { mUsed -= 1; return mUsed == 0; }

  bool isCreated() const
    { return not mBackendData; }

  SensorTime mSensorTime;
  ObsData_p mBackendData;

  typedef VersionedValue<float, Helpers::float_eq> VersionedCorrected_t;
  typedef VersionedValue<kvalobs::kvControlInfo>   VersionedControlinfo_t;
  typedef VersionedValue<std::string>              VersionedCfailed_t;

  VersionedCorrected_t   mCorrected;
  VersionedControlinfo_t mControlinfo;
  VersionedCfailed_t     mCfailed;
  size_t mUsed;
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

typedef std::set<EditVersions_p> EditVersions_ps;

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

  EditAccessPrivate_P mAccess;
  ObsRequest_p mWrapped;
  EditVersions_ps mUsed;
};

typedef EditRequest* EditRequest_P;
HQC_TYPEDEF_PV(EditRequest);
HQC_TYPEDEF_PS(EditRequest);

// ========================================================================

class EditAccessPrivate {
public:
  EditAccessPrivate(ObsAccess_p backend);

  EditVersions_ps::iterator findEditVersions(const SensorTime& st);
  ObsData_pv replace(EditRequest_P er, ObsData_pv backendData);

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
