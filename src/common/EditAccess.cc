
#include "EditAccess.hh"
#include "EditAccessPrivate.hh"

#include "DistributeUpdates.hh"
#include "KvHelpers.hh"
#include "ObsAccept.hh"

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include <boost/range/adaptor/map.hpp>

#include <stdexcept>

#define MILOGGER_CATEGORY "kvhqc.EditAccess"
#include "common/ObsLogging.hh"

namespace /*anonymous*/ {

} // namespace anonymous

EditVersions::EditVersions(ObsData_p backendData, size_t editVersion, ObsData_p editData)
  : mCurrent(1)
{
  mVersions.push_back(Version(0, backendData));
  mVersions.push_back(Version(editVersion, editData));
}

void EditVersions::addVersion(size_t version, ObsData_p data)
{
  if (version >= currentVersion()) {
    Version_v::iterator b = mVersions.begin();
    std::advance(b, mCurrent);
    mVersions.erase(b, mVersions.end());
  }
  mVersions.push_back(Version(version, data));
  mCurrent = mVersions.size()-1;
}

void EditVersions::setVersion(size_t version)
{
  METLIBS_LOG_SCOPE(LOGVAL(currentVersion()) << LOGVAL(version) << LOGVAL(mCurrent));
  while (mCurrent > 0 and version < currentVersion())
    mCurrent -= 1;
  const size_t maxVersion = mVersions.size()-1;
  while (mCurrent < maxVersion and version > currentVersion())
    mCurrent += 1;
  METLIBS_LOG_DEBUG(LOGVAL(mCurrent));
}

ObsData_p EditVersions::versionData(size_t version) const
{
  // FIXME binary search
  for (Version_v::const_iterator it = mVersions.begin(); it != mVersions.end(); ++it)
    if (it->version == version)
      return it->data;
  return ObsData_p();
}

void EditVersions::updateBackend(ObsData_p backendData)
{
  mVersions.front().data = backendData;
}

ObsData_p EditVersions::currentData() const
{
  ObsData_p c = mVersions.at(mCurrent).data;
  if (not c) {
    HQC_LOG_WARN("null current version, returning fake data");
    c = boost::make_shared<KvalobsData>(Helpers::getMissingKvData(sensorTime()), true);
  }
  return c;
}

// ========================================================================

EditRequest::EditRequest(EditAccessPrivate_P a, ObsRequest_p wrapped)
  : mAccess(a)
  , mWrapped(wrapped)
{
  mWrapped->setTag(this);
}

EditRequest_p EditRequest::untag(ObsRequest_p wrapped)
{
  EditRequest_x er = static_cast<EditRequest_x>(wrapped->tag());
  return boost::static_pointer_cast<EditRequest>(er->shared_from_this());
}

void EditRequest::completed(bool failed)
{
  mWrapped->completed(failed);
}

void EditRequest::newData(const ObsData_pv& data)
{
  mAccess->handleBackendNew(mWrapped, data);
}

void EditRequest::updateData(const ObsData_pv& data)
{
  mAccess->handleBackendUpdate(mWrapped, data);
}

void EditRequest::dropData(const SensorTime_v& dropped)
{
  mAccess->handleBackendDrop(mWrapped, dropped);
}

// ========================================================================

EditAccessPrivate::EditAccessPrivate(ObsAccess_p backend)
  : mBackend(backend)
  , mVersionTimestamps(1, timeutil::now())
  , mCurrentVersion(0)
  , mUpdated(0)
{
}

EditVersions_ps::iterator EditAccessPrivate::findEditVersions(const SensorTime& st)
{
  EditVersions_ps::iterator it = std::lower_bound(mEdited.begin(), mEdited.end(), st, lt_EditVersions());
  if (it != mEdited.end() and eq_SensorTime()((*it)->sensorTime(), st))
    return it;
  else
    return mEdited.end();
}

void EditAccessPrivate::handleBackendNew(ObsRequest_p wr, const ObsData_pv& backendData)
{
  METLIBS_LOG_SCOPE();

  ObsData_pv wrappedData;
  for (ObsData_pv::const_iterator itB = backendData.begin(); itB != backendData.end(); ++itB) {
    // TODO backendData is sorted, mEdited too => avoid some searching
    EditVersions_ps::iterator itE = findEditVersions((*itB)->sensorTime());
    if (itE != mEdited.end()) {
      EditVersions_p ev = *itE;
      if (not ev->hasBackendData())
        // FIXME there may be multiple equivalent KvalobsData objects for the same SensorTime
        ev->updateBackend(*itB);
      if (ev->currentVersion() > 0)
        // no change, omit from wrappedData
        continue;
    }
    wrappedData.push_back(*itB);
  }
  
  if (not wrappedData.empty())
    wr->newData(wrappedData);
}

void EditAccessPrivate::handleBackendUpdate(ObsRequest_p wr, const ObsData_pv& backendData)
{
  METLIBS_LOG_SCOPE();

  ObsData_pv wrappedData;
  for (ObsData_pv::const_iterator itB = backendData.begin(); itB != backendData.end(); ++itB) {
    // TODO backendData is sorted, mEdited too => avoid some searching
    EditVersions_ps::iterator itE = findEditVersions((*itB)->sensorTime());
    if (itE != mEdited.end()) {
      EditVersions_p ev = *itE;
      if (not ev->hasBackendData())
        HQC_LOG_WARN("update for non-existend backend data!?");
      // FIXME there may be multiple equivalent KvalobsData objects for the same SensorTime
      ev->updateBackend(*itB);
      if (ev->currentVersion() > 0)
        // no change, omit from wrappedData
        continue;
    }
    wrappedData.push_back(*itB);
  }
  
  if (not wrappedData.empty())
    wr->updateData(wrappedData);
}

void EditAccessPrivate::handleBackendDrop(ObsRequest_p wr, const SensorTime_v& backendDrop)
{
  METLIBS_LOG_SCOPE();

  SensorTime_v wrappedDrop;
  for (SensorTime_v::const_iterator itB = backendDrop.begin(); itB != backendDrop.end(); ++itB) {
    // TODO backendData is sorted, mEdited too => avoid some searching
    EditVersions_ps::iterator itE = findEditVersions(*itB);
    if (itE != mEdited.end()) {
      EditVersions_p ev = *itE;
      if (not ev->hasBackendData())
        HQC_LOG_WARN("drop for non-existend backend data!?");
      // FIXME there may be multiple equivalent KvalobsData objects for the same SensorTime
      ev->updateBackend(KvalobsData_p());
      if (ev->currentVersion() > 0)
        // no change, omit from wrappedData
        continue;
    }
    wrappedDrop.push_back(*itB);
  }
  
  if (not wrappedDrop.empty())
    wr->dropData(wrappedDrop);
}

void EditAccessPrivate::updateToPreviousVersion()
{
  METLIBS_LOG_SCOPE();
  mCurrentVersion -= 1;
  
  DistributeRequestUpdates<EditRequest_ps, EditRequestUnwrap> du(mRequests);

  for (EditVersions_ps::iterator itE = mEdited.begin(); itE != mEdited.end(); ++itE) {
    EditVersions_p ev = *itE;
    const size_t c0 = ev->currentVersion();
    ev->setVersion(mCurrentVersion);
    const size_t c1 = ev->currentVersion();
    METLIBS_LOG_DEBUG(LOGVAL(c0) << LOGVAL(c1) << LOGVAL(ev->hasBackendData()));
    if (c1 != c0) {
      if (c1 == 0 and not ev->hasBackendData())
        du.dropData(ev->sensorTime());
      else
        du.updateData(ev->currentData());
    }
  }

  du.send();
}

void EditAccessPrivate::updateToNextVersion()
{
  METLIBS_LOG_SCOPE();
  mCurrentVersion += 1;
  
  DistributeRequestUpdates<EditRequest_ps, EditRequestUnwrap> du(mRequests);

  for (EditVersions_ps::iterator itE = mEdited.begin(); itE != mEdited.end(); ++itE) {
    EditVersions_p ev = *itE;
    const size_t c0 = ev->currentVersion();
    ev->setVersion(mCurrentVersion);
    const size_t c1 = ev->currentVersion();
    METLIBS_LOG_DEBUG(LOGVAL(c0) << LOGVAL(c1) << LOGVAL(ev->hasBackendData()));
    if (c1 != c0) {
      if (c0 == 0 and not ev->hasBackendData())
        du.newData(ev->currentData());
      else
        du.updateData(ev->currentData());
    }
  }

  du.send();
}

// ========================================================================

EditAccess::EditAccess(ObsAccess_p backend)
  : p(new EditAccessPrivate(backend))
{
}

EditAccess::~EditAccess()
{
  if (not p->mRequests.empty())
    HQC_LOG_ERROR("destroying EditAccess with requests");
}

void EditAccess::postRequest(ObsRequest_p request)
{
  EditRequest_p r = boost::make_shared<EditRequest>(p.get(), request);
  p->mRequests.insert(r);
  p->mBackend->postRequest(r);
}

void EditAccess::dropRequest(ObsRequest_p request)
{
  METLIBS_LOG_SCOPE();
  EditRequest_p r = EditRequest::untag(request);
  if (not r)
    HQC_LOG_ERROR("not an EditRequest");
  p->mBackend->dropRequest(r);
  p->mRequests.erase(r);
}

ObsUpdate_p EditAccess::createUpdate(ObsData_p data)
{
  return boost::make_shared<EditUpdate>(data);
}

ObsUpdate_p EditAccess::createUpdate(const SensorTime& sensorTime)
{
  return boost::make_shared<EditUpdate>(sensorTime);
}

static KvalobsData_p makeData(const SensorTime& st, const timeutil::ptime& tbtime, float original,
    float co, const kvalobs::kvControlInfo& ci, const std::string& cf, bool created)
{
  const Sensor& s = st.sensor;
  kvalobs::kvUseInfo ui;
  ui.setUseFlags(ci);
  const kvalobs::kvData kvdata(s.stationId, st.time, original, s.paramId,
      tbtime, s.typeId, s.sensor, s.level, co, ci, ui, cf);
  return boost::make_shared<KvalobsData>(kvdata, created);
}

static KvalobsData_p createdData(const SensorTime& st, const timeutil::ptime& tbtime,
    float co, const kvalobs::kvControlInfo& ci, const std::string& cf)
{
  return makeData(st, tbtime, kvalobs::MISSING, co, ci, cf, true);
}

static KvalobsData_p modifiedData(ObsData_p base, float co, const kvalobs::kvControlInfo& ci, const std::string& cf)
{
  return makeData(base->sensorTime(), base->tbtime(), base->original(), co, ci, cf, false);
}

bool EditAccess::storeUpdates(const ObsUpdate_pv& updates)
{
  METLIBS_LOG_SCOPE();

  DistributeRequestUpdates<EditRequest_ps, EditRequestUnwrap> du(p->mRequests);

  const timeutil::ptime& tbtime = versionTimestamp(currentVersion());

  BOOST_FOREACH(ObsUpdate_p u, updates) {
    EditUpdate_p eu = boost::dynamic_pointer_cast<EditUpdate>(u);
    if (not eu) {
      // FIXME HCQ_LOG_ERROR("not an EditUpdate:" << u->sensorTime());
      continue;
    }

    EditVersions_ps::iterator itE = p->findEditVersions(eu->sensorTime());
    if (itE != p->mEdited.end()) {
      // TODO check for correct parent
      KvalobsData_p d = modifiedData((*itE)->currentData(), eu->corrected(), eu->controlinfo(), eu->cfailed());
      METLIBS_LOG_DEBUG("edited before" << LOGOBS(d));
      (*itE)->addVersion(p->mCurrentVersion, d);
      du.updateData((*itE)->currentData());
    } else {
      KvalobsData_p d;
      if (eu->obs())
        d = modifiedData(eu->obs(), eu->corrected(), eu->controlinfo(), eu->cfailed());
      else
        d = createdData(eu->sensorTime(), tbtime, eu->corrected(), eu->controlinfo(), eu->cfailed());
      METLIBS_LOG_DEBUG("new edit" << LOGOBS(d));
      EditVersions_p ev = boost::make_shared<EditVersions>(eu->obs(), p->mCurrentVersion, d);
      p->mEdited.insert(ev);
      if (eu->obs())
        du.updateData(ev->currentData());
      else
        du.newData(ev->currentData());
      METLIBS_LOG_DEBUG(" => currentData = " << LOGOBS(ev->currentData()));
    }
  }

  du.send();

  return true;
}

template<class T>
class swapback {
public:
  swapback(T& old, const T& nu = T())
    : mOld(old), mNew(nu), mAccept(false) { swap(); }
  void accept()
    { mAccept = true; }
  void swap()
    { std::swap(mOld, mNew); }
  T& value()
    { return mNew; }
  
  ~swapback()
    { if (not mAccept) swap(); }
private:
  T& mOld;
  T mNew;
  bool mAccept;
};

bool EditAccess::storeToBackend()
{
  swapback<EditVersions_ps> oldEdited(p->mEdited);
  swapback<size_t> oldCurrent(p->mCurrentVersion, 0);

  ObsUpdate_pv updates;
  BOOST_FOREACH(EditVersions_p ev, oldEdited.value()) {
    ObsUpdate_p backendUpdate;
    if (ev->hasBackendData())
      backendUpdate = p->mBackend->createUpdate(ev->backendData());
    else
      backendUpdate = p->mBackend->createUpdate(ev->sensorTime());
    backendUpdate->setCorrected  (ev->corrected());
    backendUpdate->setControlinfo(ev->controlinfo());
    backendUpdate->setCfailed    (ev->cfailed());
    updates.push_back(backendUpdate);
  }
  if (p->mBackend->storeUpdates(updates)) {
    oldEdited.accept();
    oldCurrent.accept();

    if (p->mVersionTimestamps.size() > 1)
      p->mVersionTimestamps.erase(p->mVersionTimestamps.begin() + 1, p->mVersionTimestamps.end());
    Q_EMIT currentVersionChanged(p->mCurrentVersion, p->mCurrentVersion);
    
    return true;
  }
  return false;
}

void EditAccess::reset()
{
  METLIBS_LOG_SCOPE();

  DistributeRequestUpdates<EditRequest_ps, EditRequestUnwrap> du(p->mRequests);

  for (EditVersions_ps::iterator itE = p->mEdited.begin(); itE != p->mEdited.end(); ++itE) {
    EditVersions_p ev = *itE;
    if (ev->hasBackendData())
      du.updateData(ev->backendData());
    else
      du.dropData(ev->sensorTime());
  }

  p->mCurrentVersion = 0;
  p->mEdited.clear();
  if (p->mVersionTimestamps.size() > 1)
    p->mVersionTimestamps.erase(p->mVersionTimestamps.begin() + 1, p->mVersionTimestamps.end());
  Q_EMIT currentVersionChanged(p->mCurrentVersion, p->mCurrentVersion);

  du.send();
}

void EditAccess::newVersion()
{
  METLIBS_LOG_SCOPE();
  p->mCurrentVersion += 1;
  if (p->mVersionTimestamps.size() >= p->mCurrentVersion)
    p->mVersionTimestamps.erase(p->mVersionTimestamps.begin() + p->mCurrentVersion, p->mVersionTimestamps.end());
  p->mVersionTimestamps.push_back(timeutil::now());
}

bool EditAccess::canUndo() const
{
  return p->mCurrentVersion > 0;
}

bool EditAccess::canRedo() const
{
  return p->mCurrentVersion < highestVersion();
}

size_t EditAccess::highestVersion() const
{
  return p->mVersionTimestamps.size() - 1;
}

size_t EditAccess::currentVersion() const
{
  return p->mCurrentVersion;
}

size_t EditAccess::countU() const
{
  return p->mUpdated;
}

void EditAccess::undoVersion()
{
  METLIBS_LOG_SCOPE();
  if (p->mCurrentVersion > 0)
    p->updateToPreviousVersion();
}

void EditAccess::redoVersion()
{
  METLIBS_LOG_SCOPE();
  if (p->mCurrentVersion < highestVersion())
    p->updateToNextVersion();
}

const timeutil::ptime& EditAccess::versionTimestamp(size_t version) const
{
  return p->mVersionTimestamps[version];
}

ObsData_pv EditAccess::versionChanges(size_t version) const
{
  ObsData_pv haveVersion;
  BOOST_FOREACH(EditVersions_p ev, p->mEdited) {
    if (ObsData_p d = ev->versionData(version))
      haveVersion.push_back(d);
  }
  return haveVersion;
}
