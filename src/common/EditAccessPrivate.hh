
#ifndef ACCESS_EDITACCESSPRIVATE_HH
#define ACCESS_EDITACCESSPRIVATE_HH 1

#include "EditAccess.hh"
#include "KvalobsData.hh"
#include "WrapRequest.hh"

#include "util/VersionedValue.hh"
#include "common/Functors.hh"

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

class EditRequest : public WrapRequest {
public:
  EditRequest(EditAccessPrivate_P a, ObsRequest_p wrapped);

  static EditRequest_p untag(ObsRequest_p wrapped);
  
  virtual void newData(const ObsData_pv& data);
  virtual void updateData(const ObsData_pv& data);
  virtual void dropData(const SensorTime_v& dropped);

  EditAccessPrivate_P mAccess;
};

HQC_TYPEDEF_X(EditRequest);
HQC_TYPEDEF_PV(EditRequest);
HQC_TYPEDEF_PS(EditRequest);

struct EditRequestUnwrap {
  ObsRequest_p operator()(EditRequest_p er) const
    { return er->wrapped(); }
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
