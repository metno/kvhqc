
#include "EditAccess.hh"
#include "EditAccessPrivate.hh"

#include "DistributeUpdates.hh"
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

EditVersions::EditVersions(ObsData_p backendData)
  : mSensorTime(backendData->sensorTime())
  , mBackendData(backendData)
  , mCorrected(backendData->corrected())
  , mControlinfo(backendData->controlinfo())
  , mCfailed(backendData->cfailed())
  , mUsed(0)
{
}

EditVersions::EditVersions(const SensorTime& st)
  : mSensorTime(st)
  , mCorrected(-32768)
  , mControlinfo(std::string("0000000000000000"))
  , mCfailed("hqc-new")
  , mUsed(0)
{
}

bool EditVersions::set(size_t version, bool drop)
{
  bool changed = false;
  changed |= mCorrected  .setVersion(version, drop);
  changed |= mControlinfo.setVersion(version, drop);
  changed |= mCfailed    .setVersion(version, drop);
  return changed;
}

bool EditVersions::hasVersion(size_t version) const
{
  return mCorrected  .hasVersion(version)
      or mControlinfo.hasVersion(version)
      or mCfailed    .hasVersion(version);
}

bool EditVersions::set(size_t version, float corrected, const kvalobs::kvControlInfo& controlinfo, const std::string& cfailed)
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
  bool changedCO, changedCI, changedCF;
  if (mBackendData) {
    changedCO = mCorrected  .changeOriginal(mBackendData->corrected());
    changedCI = mControlinfo.changeOriginal(mBackendData->controlinfo());
    changedCF = mCfailed    .changeOriginal(mBackendData->cfailed());
  } else {
    changedCO = mCorrected  .changeOriginal(-32768);
    changedCI = mControlinfo.changeOriginal(std::string("0000000000000000"));
    changedCF = mCfailed    .changeOriginal("");
  }

  METLIBS_LOG_DEBUG("changed co=" << changedCO << " ci=" << changedCI << " cf=" << changedCF);

  return (changedCO or changedCI or changedCF);
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
  const ObsData_pv replaced = mAccess->replace(this, data);
  mWrapped->newData(replaced);
}

void EditRequest::updateData(const ObsData_pv& data)
{
  METLIBS_LOG_SCOPE();
  // this is called from the backend
  const ObsData_pv replaced = mAccess->replace(this, data);
  mWrapped->updateData(replaced);
}

void EditRequest::dropData(const SensorTime_v& dropped)
{
  mAccess->replace(this, dropped);
  mWrapped->dropData(dropped);
}

void EditRequest::use(EditVersions_p ev)
{
  if (not mUsed.count(ev)) {
    ev->use();
    mUsed.insert(ev);
  }
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

ObsData_pv EditAccessPrivate::replace(EditRequest_x er, ObsData_pv backendData)
{
  METLIBS_LOG_SCOPE();
  for (ObsData_pv::iterator itB = backendData.begin(); itB != backendData.end(); ++itB) {
    EditVersions_ps::iterator itEV = findEditVersions((*itB)->sensorTime());
    if (itEV != mEdited.end()) {
      EditVersions_p ev = *itEV;
      ev->updateBackend(*itB);
      *itB = ev;
      er->use(ev);
    }
  }
  return backendData;
}

void EditAccessPrivate::replace(EditRequest_x er, const SensorTime_v& backendDropped)
{
  METLIBS_LOG_SCOPE();
  ObsData_p none;
  for (SensorTime_v::const_iterator itST = backendDropped.begin(); itST != backendDropped.end(); ++itST) {
    EditVersions_ps::iterator itEV = findEditVersions(*itST);
    if (itEV != mEdited.end())
      (*itEV)->updateBackend(none);
  }
}

void EditAccessPrivate::updateWrapped(DistributeUpdates& du, EditVersions_p ev)
{
  BOOST_FOREACH(EditRequest_p r, mRequests) {
    if (acceptObs(r->mWrapped, ev)) {
      r->use(ev);
      du.updateData(r->mWrapped, ev);
    }
  }
}

void EditAccessPrivate::insertWrapped(DistributeUpdates& du, EditVersions_p ev)
{
  BOOST_FOREACH(EditRequest_p r, mRequests) {
    if (acceptObs(r->mWrapped, ev)) {
      r->use(ev);
      du.newData(r->mWrapped, ev);
    }
  }
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

  BOOST_FOREACH(EditVersions_p ev, r->mUsed) {
    METLIBS_LOG_DEBUG(ev->sensorTime());
    if (ev->drop())
      p->mEdited.erase(ev);
  }
  // TODO warn if an EditVersions object is modified
}

ObsUpdate_p EditAccess::createUpdate(ObsData_p data)
{
  return boost::make_shared<EditUpdate>(data);
}

ObsUpdate_p EditAccess::createUpdate(const SensorTime& sensorTime)
{
  return boost::make_shared<EditUpdate>(sensorTime);
}

bool EditAccess::storeUpdates(const ObsUpdate_pv& updates)
{
  METLIBS_LOG_SCOPE();
  DistributeUpdates du;

  BOOST_FOREACH(ObsUpdate_p u, updates) {
    EditUpdate_p eu = boost::dynamic_pointer_cast<EditUpdate>(u);
    if (not eu) {
      // FIXME HCQ_LOG_ERROR("not an EditUpdate:" << u->sensorTime());
      continue;
    }
    EditVersions_p ev;
    EditVersions_ps::iterator itEV = p->findEditVersions(eu->sensorTime());
    const bool insert = (itEV == p->mEdited.end()), haveB = eu->obs();
    if (insert) {
      if (haveB)
        ev = boost::make_shared<EditVersions>(eu->obs());
      else
        ev = boost::make_shared<EditVersions>(eu->sensorTime());
    } else {
      ev = *itEV;
    }
    if (ev->set(p->mCurrentVersion, eu->mCorrected, eu->mControlinfo, eu->mCfailed)) {
      if (insert and not haveB)
        p->insertWrapped(du, ev);
      else
        p->updateWrapped(du, ev);
    }
    METLIBS_LOG_DEBUG(LOGVAL(ev->mUsed));
    if (itEV == p->mEdited.end() and ev->mUsed > 0)
      p->mEdited.insert(ev);
  }

  du.send();

  return true;
}

bool EditAccess::storeToBackend()
{
  EditVersions_ps edited; // swap, no need to search for each update from backend
  edited.swap(p->mEdited);

  ObsUpdate_pv updates;
  BOOST_FOREACH(EditVersions_p ev, edited) {
    ObsUpdate_p backendUpdate;
    if (ev->isCreated())
      backendUpdate = p->mBackend->createUpdate(ev->mSensorTime);
    else
      backendUpdate = p->mBackend->createUpdate(ev->mBackendData);
    backendUpdate->setCorrected  (ev->corrected());
    backendUpdate->setControlinfo(ev->controlinfo());
    backendUpdate->setCfailed    (ev->cfailed());
    updates.push_back(backendUpdate);
  }
  try {
    if (not p->mBackend->storeUpdates(updates)) {
      edited.swap(p->mEdited);
      return false;
    }
  } catch (...) {
    edited.swap(p->mEdited);
    throw;
  }

  // updateData calls should already come from backend

  // FIXME when to set mCurrentVersion=0? before or while or after mBackend->storeUpdates?

  p->mCurrentVersion = 0;
  p->mEdited.clear();
  if (p->mVersionTimestamps.size() > 1)
    p->mVersionTimestamps.erase(p->mVersionTimestamps.begin() + 1, p->mVersionTimestamps.end());

  BOOST_FOREACH(EditRequest_p r, p->mRequests) {
    r->mUsed.clear();
  }

  Q_EMIT currentVersionChanged(p->mCurrentVersion, p->mCurrentVersion);

  return true;
}

void EditAccess::reset()
{
  METLIBS_LOG_SCOPE();

  p->mCurrentVersion = 0;
  p->mEdited.clear();
  if (p->mVersionTimestamps.size() > 1)
    p->mVersionTimestamps.erase(p->mVersionTimestamps.begin() + 1, p->mVersionTimestamps.end());

  BOOST_FOREACH(EditRequest_p r, p->mRequests) {
    ObsData_pv updated;
    SensorTime_v dropped;
    BOOST_FOREACH(EditVersions_p ev, r->mUsed) {
      if (ev->isCreated()) {
        METLIBS_LOG_DEBUG(LOGVAL("drop: " << ev->sensorTime()));
        dropped.push_back(ev->sensorTime());
      } else {
        METLIBS_LOG_DEBUG(LOGVAL("update: " << ev->sensorTime()));
        updated.push_back(ev->mBackendData);
      }
    }
    if (not updated.empty())
      r->mWrapped->updateData(updated);
    if (not dropped.empty())
      r->mWrapped->dropData(dropped);

    r->mUsed.clear();
  }

  Q_EMIT currentVersionChanged(p->mCurrentVersion, p->mCurrentVersion);
}

void EditAccess::newVersion()
{
  METLIBS_LOG_SCOPE();
  p->mCurrentVersion += 1;
  if (p->mVersionTimestamps.size() >= p->mCurrentVersion)
    p->mVersionTimestamps.erase(p->mVersionTimestamps.begin() + p->mCurrentVersion, p->mVersionTimestamps.end());
  p->mVersionTimestamps.push_back(timeutil::now());

  updateToCurrentVersion(true);
}

void EditAccess::updateToCurrentVersion(bool drop)
{
  METLIBS_LOG_SCOPE(LOGVAL(drop) << LOGVAL(p->mCurrentVersion));
  DistributeUpdates du;
  BOOST_FOREACH(EditVersions_p ev, p->mEdited) {
    const bool changed = ev->set(p->mCurrentVersion, drop);
    METLIBS_LOG_DEBUG(LOGVAL(ev->sensorTime()) << LOGVAL(changed) << LOGVAL(ev->corrected()));
    if (changed)
      p->updateWrapped(du, ev);
  }
  du.send();
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
  if (p->mCurrentVersion == 0)
    return;
  p->mCurrentVersion -= 1;
  updateToCurrentVersion(false);
}

void EditAccess::redoVersion()
{
  METLIBS_LOG_SCOPE();
  if (p->mCurrentVersion >= highestVersion())
    return;
  p->mCurrentVersion += 1;
  updateToCurrentVersion(false);
}

const timeutil::ptime& EditAccess::versionTimestamp(size_t version) const
{
  return p->mVersionTimestamps[version];
}

ObsData_pv EditAccess::versionChanges(size_t version) const
{
  ObsData_pv haveVersion;
  BOOST_FOREACH(EditVersions_p ev, p->mEdited) {
    if (ev->hasVersion(version))
      haveVersion.push_back(ev);
  }
  return haveVersion;
}
