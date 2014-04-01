
#include "EditAccess.hh"
#include "ObsHelpers.hh"

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include <boost/range/adaptor/map.hpp>

#include <stdexcept>

#define MILOGGER_CATEGORY "kvhqc.EditAccess"
#include "common/ObsLogging.hh"

EditVersions::EditVersions(ObsData_p backendData)
  : mBackendData(backendData)
  , mCorrected(mData->corrected())
  , mControlinfo(mData->controlinfo())
  , mCfailed(mData->cfailed())
  , mUsed(0)
{
}

bool EditVersions::setVersion(int version, bool drop)
{
  bool changed = false;
  changed |= mCorrected  .setVersion(version, drop);
  changed |= mControlinfo.setVersion(version, drop);
  changed |= mCfailed    .setVersion(version, drop);
  return changed;
}

bool EditVersions::set(int version, float corrected, const edit::kvControlInfo& controlinfo, const std::string& cfailed)
{
  bool changed = false;
  changed |= mCorrected  .setValue(version, corrected);
  changed |= mControlinfo.setValue(version, controlinfo);
  changed |= mCfailed    .setValue(version, cfailed);
  return changed;
}

bool EditVersions::updateBackend(ObsData_p backendData)
{
  METLIBS_LOG_SCOPE();

  mBackendData = backendData;
  const bool changedCO = mCorrected  .changeOriginal(mBackendData->corrected());
  const bool changedCI = mControlinfo.changeOriginal(mBackendData->controlinfo());
  const bool changedCF = mCfailed    .changeOriginal(mBackendData->cfailed());

  METLIBS_LOG_DEBUG("changed co=" << changedCO << " ci=" << changedCI << " cf=" << changedCF);

  return (changedCO or changedCI or changedCF);
}

void EditVersions::use()
{
  mUsed += 1;
}

bool EditVersions::drop()
{
  mUsed -= 1;
  return mUsed == 0;
}

// ========================================================================

class EditRequest : public ObsRequest {
public:
  EditRequest(EditAccess_P a, ObsRequest_p wrapped)
    : mAccess(a), mWrapped(wrapped) { }
  
  virtual const Sensor_s& sensors() const
    { return mWrapped->sensors(); }

  virtual const TimeSpan& timeSpan() const
    { return mWrapped->timeSpan(); }

  virtual ObsFilter_p filter() const
    { return mWrapped->filter(); }

  virtual void completed(bool failed)
    { mWrapped->completed(failed); }
  virtual void newData(const ObsData_pv& data)
    { mWrapped->newData(use(mAccess->replace(data))); }
  virtual void updateData(const ObsData_pv& data)
    { mWrapped->updateData(use(mAccess->replace(data))); }
  virtual void dropData(const SensorTime_v& dropped)
    { mWrapped->newData(mAccess->replace(dropped)); }

  EditVersions_pv use(EditVersions_pv evs);
  EditVersions_s drop();

  EditAccess_P mAccess;
  ObsRequest_p mWrapped;
  EditVersions_s mUsed;
};

EditVersions_pv EditRequest::use(EditVersions_pv evs)
{
  BOOST_FOREACH(EditVersions_p ev, evs) {
    if (not mUsed.count(ev))
      ev->use();
  }
  return evs;
}

EditVersions_s EditRequest::drop()
{
  EditVersions_s unused;
  BOOST_FOREACH(EditVersions& ev, mUsed) {
    if (ev->drop())
      unused.insert(ev);
  }
  return unused;
}

// ========================================================================

EditAccess::EditAccess(ObsAccess_p backend)
  : mBackend(backend)
  , mVersionTimestamps(1, timeutil::now())
  , mCurrentVersion(0)
  , mUpdated(0)
{
}

EditAccess::~EditAccess()
{
  if (not mRequests.empty())
    HQC_LOG_ERROR("destroying EditAccess with requests");
}

void EditAccess::postRequest(ObsRequest_p request)
{
  EditRequest_p r = boost::make_shared<EditRequest>(this, request);
  mRequests.push_back(r);
  mBackend.postRequest(r);
}

void EditAccess::dropRequest(ObsRequest_p request)
{
  EditRequest_p r = boost::static_pointer_cast<EditRequest>(request);
  mBackend->dropRequest(r->mWrapped);
  mRequests.erase(r);

  set<SensorTime> unused = r->drop();
  mSensorEditVersions.eraseAll(unused);
  // TODO warn if an EditVersions object is modified
}

ObsData_pv EditAccess::replace(ObsData_pv backendData)
{
  for (ObsData_pv::iterator it = backendData.begin(); it != backendData.end(); ++it) {
    if (mSensorEditVersions.find((*it)->sensorTime())) {
      EditVersions_p& ev = *mSensorEditVersions[(*it)->sensorTime()];
      *it = ev_p;
    }
  }
  return backendData;
}

ObsUpdate_p EditAccess::createUpdate(ObsData_p& data)
{
  return boost::make_shared<EditUpdate>(data);
}

ObsUpdate_p EditAccess::createUpdate(const SensorTime& sensorTime)
{
  KvalobsData_p d = boost::make_shared<KvalobsData>(Helpers::getMissingKvData(sensorTime), true);
  return boost::make_shared<EditUpdate>(d);
}

bool EditAccess::storeUpdates(const ObsUpdate_pv& updates)
{
  BOOST_FOREACH(ObsUpdate_p u, updates) {
    EditUpdate_p eu = boost::dynamic_pointer_cast<EditUpdate>(u);
    if (not eu) {
      HCQ_LOG_ERROR("not an EditUpdate:" << u->sensorTime());
      continue;
    }
    EditVersions& ev = findEditVersions(eu->sensorTime());
    ev->set(mCurrentVersion, eu->corrected(), eu->controlinfo(), eu->cfailed());

    BOOST_FOREACH(ObsRequest_p r, mRequests) {
      if (r matches eu->sensorTime()) {
        r->updateData(...);
      }
    }
  }
}

bool EditAccess::storeToBackend()
{
  SensorEditVersions sev; // swap, no need to search for each update from backend
  sev.swap(mSensorEditVersions);

  ObsUpdate_pv updates;
  BOOST_FOREACH(const SensorEditVersions::value_type& sevv, sev) {
    const EditVersions& ev = sevv.second;
    ObsUpdate_p backendUpdate;
    if (sev->isCreated())
      backendUpdate = mBackend->createUpdate(ev.backendData());
    else
      backendUpdate = mBackend->createUpdate(ev.sensorTime());
    backendUpdate->setCorrected  (ev.corrected());
    backendUpdate->setControlinfo(ev.controlinfo());
    backendUpdate->setCfailed    (ev.cfailed());
    updates.push_back(backendUpdate);
  }
  try {
    if (not mBackend->storeUpdates(updates)) {
      sev.swap(mSensorEditVersions);
      return false;
    }
  } catch (...) {
    sev.swap(mSensorEditVersions);
    throw;
  }

  // updateData calls should already come from backend

  // FIXME when to set mCurrentVersion=0? before or while or after mBackend->storeUpdates?

  mCurrentVersion = 0;
  Q_EMIT currentVersionChanged(mCurrentVersion, mCurrentVersion);
}

void EditAccess::reset()
{
  BOOST_FOREACH(const SensorEditVersions::value_type& sevv, sev) {
    BOOST_FOREACH(ObsRequest_p r, mRequests) {
      if (r matches eu->sensorTime()) {
        r->updateData(...);
      }
    }
  }

  mCurrentVersion = 0;
  mSensorEditVersions.clear();
  if (mVersionTimestamps.size() > 1)
    mVersionTimestamps.erase(mVersionTimestamps.begin() + 1, mVersionTimestamps.end());
  Q_EMIT currentVersionChanged(mCurrentVersion, mCurrentVersion);
}

void EditAccess::newVersion()
{
  METLIBS_LOG_SCOPE();
  mCurrentVersion += 1;
  if ((int)mVersionTimestamps.size() >= mCurrentVersion)
    mVersionTimestamps.erase(mVersionTimestamps.begin() + mCurrentVersion, mVersionTimestamps.end());
  mVersionTimestamps.push_back(timeutil::now());

  updateToCurrentVersion(true);
}

void EditAccess::updateToCurrentVersion(bool drop)
{
  std::vector<PopUpdate> updates;
  BOOST_FOREACH(const SensorEditVersions::value_type& sevv, sev) {
  BOOST_FOREACH(EditDataPtr ebs, mData | boost::adaptors::map_values) {
    if (not ebs)
      continue;
    const int wasUpdated = ebs->modified()?1:0, hadTasks = ebs->hasRequiredTasks()?1:0;
    bool changed = ebs->mCorrected.setVersion(mCurrentVersion, drop);
    changed |= ebs->mControlinfo.setVersion(mCurrentVersion, drop);
    changed |= ebs->mTasks.setVersion(mCurrentVersion, drop);

    if (changed) {
      const int isUpdated = (ebs->modified())?1:0, hasTasks = ebs->hasRequiredTasks()?1:0;
      updates.push_back(PopUpdate(ebs, isUpdated - wasUpdated, hasTasks - hadTasks));
    }
  }

  /* emit */ currentVersionChanged(currentVersion(), highestVersion());
  BOOST_FOREACH(const PopUpdate& p, updates)
      sendObsDataChanged(MODIFIED, p.ebs, p.dU, p.dT);
}

void EditAccess::undoVersion()
{
  METLIBS_LOG_SCOPE();
  if (mCurrentVersion == 0)
    return;
  mCurrentVersion -= 1;
  updateToCurrentVersion(false);
}

void EditAccess::redoVersion()
{
  METLIBS_LOG_SCOPE();
  if (mCurrentVersion >= highestVersion())
    return;
  mCurrentVersion += 1;
  updateToCurrentVersion(false);
}

EditAccess::ChangedData_t EditAccess::versionChanges(int version) const
{
  ChangedData_t haveVersion;
  BOOST_FOREACH(EditDataPtr ebs, mData | boost::adaptors::map_values) {
    if (ebs and ebs->hasVersion(version))
      haveVersion.push_back(ebs);
  }
  return haveVersion;
}

void EditAccess::onBackendDataChanged(ObsAccess::ObsDataChange what, ObsDataPtr obs)
{
  METLIBS_LOG_SCOPE();
  METLIBS_LOG_DEBUG(LOGVAL(what) << LOGOBS(obs));

  EditDataPtr ebs = findE(SensorTime(obs->sensorTime()));
  if (not ebs)
    return;

  const int wasModified = ebs->modified()?1:0, hadTasks = ebs->hasRequiredTasks()?1:0;
  const bool backendChanged = ebs->updateFromBackend();
  METLIBS_LOG_DEBUG(LOGVAL(backendChanged));
  if (backendChanged) {
    const int isModified = ebs->modified()?1:0, hasTasks = ebs->hasRequiredTasks()?1:0;
    sendObsDataChanged(EditAccess::MODIFIED, ebs, isModified - wasModified, hasTasks - hadTasks);
    backendDataChanged(what, ebs);
  }
}
