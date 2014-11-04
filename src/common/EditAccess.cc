
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
  : mSensorTime(editData->sensorTime())
  , mCurrent(1)
{
  METLIBS_LOG_SCOPE();
  mVersions.push_back(Version(0, backendData));
  mVersions.push_back(Version(editVersion, editData));
  METLIBS_LOG_DEBUG(LOGVAL(sensorTime()));
}

void EditVersions::addVersion(size_t version, ObsData_p data)
{
  METLIBS_LOG_SCOPE(LOGVAL(sensorTime()) << LOGVAL(currentVersion()) << LOGVAL(version));
  if (version >= currentVersion() and mCurrent < mVersions.size() - 1) {
    Version_v::iterator b = mVersions.begin();
    std::advance(b, mCurrent+1);
    mVersions.erase(b, mVersions.end());
  }
  mVersions.push_back(Version(version, data));
  mCurrent = mVersions.size()-1;
}

void EditVersions::setVersion(size_t version)
{
  METLIBS_LOG_SCOPE(LOGVAL(sensorTime()) << LOGVAL(currentVersion()) << LOGVAL(version) << LOGVAL(mCurrent));
  if (version < currentVersion()) {
    while (mCurrent > 0 and version < currentVersion())
      mCurrent -= 1;
  } else {
    const size_t maxVersion = mVersions.size()-1;
    while (mCurrent < maxVersion and version >= mVersions[mCurrent+1].version)
      mCurrent += 1;
  }
  METLIBS_LOG_DEBUG(LOGVAL(mCurrent) << LOGVAL(currentVersion()));
}

void EditVersions::dropVersionsFrom(size_t version)
{
  METLIBS_LOG_SCOPE(LOGVAL(sensorTime()) << LOGVAL(currentVersion()) << LOGVAL(version) << LOGVAL(mCurrent));
  const bool newCurrent = (version <= currentVersion());
  Version_v::iterator b = mVersions.begin();
  while (b != mVersions.end() and b->version < version)
    ++b;
  mVersions.erase(b, mVersions.end());
  if (newCurrent)
    mCurrent = mVersions.size() - 1;
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
  : WrapRequest(wrapped)
  , mAccess(a)
{
  mAccess->handleBackendEdited(wrapped);
}

EditRequest_p EditRequest::untag(ObsRequest_p wrapped)
{
  return boost::static_pointer_cast<EditRequest>(WrapRequest::untag(wrapped));
}

void EditRequest::newData(const ObsData_pv& data)
{
  mAccess->handleBackendNew(wrapped(), data);
}

void EditRequest::updateData(const ObsData_pv& data)
{
  mAccess->handleBackendUpdate(wrapped(), data);
}

void EditRequest::dropData(const SensorTime_v& dropped)
{
  mAccess->handleBackendDrop(wrapped(), dropped);
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

void EditAccessPrivate::handleBackendEdited(ObsRequest_p wr)
{
  METLIBS_LOG_SCOPE();

  const Sensor_s& rsensors = wr->sensors();
  const TimeSpan& rtime    = wr->timeSpan();
  ObsFilter_p     rfilter  = wr->filter();

  ObsData_pv editedData;
  for (EditVersions_ps::const_iterator it = mEdited.begin(); it != mEdited.end(); ++it) {
    EditVersions_p ev = *it;
    if (not rtime.contains(ev->sensorTime().time))
      continue;
    if (not rsensors.count(ev->sensorTime().sensor))
      continue;

    ObsData_p d = ev->currentData();
    if (not rfilter or rfilter->accept(d, false))
      editedData.push_back(d);
  }

  if (not editedData.empty())
    wr->newData(editedData);
}

void EditAccessPrivate::handleBackendNew(ObsRequest_p wr, const ObsData_pv& backendData)
{
  METLIBS_LOG_SCOPE();

  ObsData_pv wrappedData;
  for (ObsData_pv::const_iterator itB = backendData.begin(); itB != backendData.end(); ++itB) {
    // TODO backendData is sorted, mEdited too => avoid some searching
    if (findEditVersions((*itB)->sensorTime()) == mEdited.end())
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
    METLIBS_LOG_DEBUG(LOGVAL(ev->sensorTime()) << LOGVAL(c0) << LOGVAL(c1) << LOGVAL(ev->hasBackendData()));
    if (c1 != c0) {
      if (c1 == 0 and not ev->hasBackendData())
        du.dropData(ev->sensorTime());
      else
        du.updateData(ev->currentData());
      if (c1 == 0)
        mUpdated -= 1;
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
    METLIBS_LOG_DEBUG(LOGVAL(ev->sensorTime()) << LOGVAL(c0) << LOGVAL(c1) << LOGVAL(ev->hasBackendData()));
    if (c1 != c0) {
      if (c0 == 0 and not ev->hasBackendData())
        du.newData(ev->currentData());
      else
        du.updateData(ev->currentData());
      if (c0 == 0)
        mUpdated += 1;
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
  reset();
  if (not p->mRequests.empty())
    HQC_LOG_ERROR("destroying EditAccess with requests");
}

void EditAccess::postRequest(ObsRequest_p request, bool synchronized)
{
  EditRequest_p r = boost::make_shared<EditRequest>(p.get(), request);
  p->mRequests.insert(r);
  p->mBackend->postRequest(r, synchronized);
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
  return boost::make_shared<KvalobsUpdate>(data);
}

ObsUpdate_p EditAccess::createUpdate(const SensorTime& sensorTime)
{
  return boost::make_shared<KvalobsUpdate>(sensorTime);
}

bool EditAccess::storeUpdates(const ObsUpdate_pv& updates)
{
  METLIBS_LOG_SCOPE();

  DistributeRequestUpdates<EditRequest_ps, EditRequestUnwrap> du(p->mRequests);

  const timeutil::ptime& tbtime = versionTimestamp(currentVersion());

  for (ObsUpdate_pv::const_iterator it = updates.begin(); it != updates.end(); ++it) {
    KvalobsUpdate_p eu = boost::static_pointer_cast<KvalobsUpdate>(*it);
    METLIBS_LOG_DEBUG(LOGVAL(eu->sensorTime()));

    EditVersions_ps::iterator itE = p->findEditVersions(eu->sensorTime());
    if (itE != p->mEdited.end()) {
      // TODO check for correct parent
      //assert((*itE)->currentData() == eu->obs());
      KvalobsData_p d = createDataForUpdate(eu, tbtime);
      EditVersions_p ev = *itE;
      if (ev->currentVersion() == 0)
        p->mUpdated += 1;
      ev->addVersion(p->mCurrentVersion, d);
      du.updateData(ev->currentData());
    } else {
      KvalobsData_p d = createDataForUpdate(eu, tbtime);
      d->setModified(true);
      EditVersions_p ev = boost::make_shared<EditVersions>(eu->obs(), p->mCurrentVersion, d);
      p->mEdited.insert(ev);
      if (eu->obs())
        du.updateData(ev->currentData());
      else
        du.newData(ev->currentData());
      p->mUpdated += 1;
    }
  }

  du.send();

  Q_EMIT currentVersionChanged(currentVersion(), currentVersion());

  return true;
}

KvalobsData_p EditAccess::createDataForUpdate(KvalobsUpdate_p eu, const timeutil::ptime& tbtime)
{
  if (eu->obs())
    return Helpers::modifiedData(eu->obs(), eu->corrected(), eu->controlinfo(), eu->cfailed());
  else
    return Helpers::createdData(eu->sensorTime(), tbtime, eu->corrected(), eu->controlinfo(), eu->cfailed());
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
  swapback<size_t> oldCurrent(p->mCurrentVersion, 0), oldUpdated(p->mUpdated, 0);

  ObsUpdate_pv updates;
  BOOST_FOREACH(EditVersions_p ev, oldEdited.value()) {
    ObsUpdate_p backendUpdate;
    if (ev->hasBackendData())
      backendUpdate = p->mBackend->createUpdate(ev->backendData());
    else
      backendUpdate = p->mBackend->createUpdate(ev->sensorTime());
    if (not fillBackendupdate(backendUpdate, ev->currentData()))
      return false;
    updates.push_back(backendUpdate);
  }
  if (p->mBackend->storeUpdates(updates)) {
    oldEdited.accept();
    oldCurrent.accept();
    oldUpdated.accept();

    if (p->mVersionTimestamps.size() > 1)
      p->mVersionTimestamps.erase(p->mVersionTimestamps.begin() + 1, p->mVersionTimestamps.end());
    Q_EMIT currentVersionChanged(currentVersion(), currentVersion());
    
    return true;
  }
  return false;
}

bool EditAccess::fillBackendupdate(ObsUpdate_p backendUpdate, ObsData_p currentData)
{
  backendUpdate->setCorrected  (currentData->corrected());
  backendUpdate->setControlinfo(currentData->controlinfo());
  backendUpdate->setCfailed    (currentData->cfailed());
  return true;
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
  p->mUpdated = 0;

  Q_EMIT currentVersionChanged(currentVersion(), currentVersion());

  du.send();
}

void EditAccess::newVersion()
{
  METLIBS_LOG_SCOPE();
  p->mCurrentVersion += 1;
  if (p->mVersionTimestamps.size() >= p->mCurrentVersion)
    p->mVersionTimestamps.erase(p->mVersionTimestamps.begin() + p->mCurrentVersion, p->mVersionTimestamps.end());
  p->mVersionTimestamps.push_back(timeutil::now());

  for (EditVersions_ps::iterator itE = p->mEdited.begin(); itE != p->mEdited.end(); ++itE)
    (*itE)->dropVersionsFrom(p->mCurrentVersion);

  Q_EMIT currentVersionChanged(currentVersion(), currentVersion());
}

bool EditAccess::canUndo() const
{
  return currentVersion() > 0;
}

bool EditAccess::canRedo() const
{
  return currentVersion() < highestVersion();
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
  if (canUndo()) {
    p->updateToPreviousVersion();
    Q_EMIT currentVersionChanged(currentVersion(), highestVersion());
  }
}

void EditAccess::redoVersion()
{
  METLIBS_LOG_SCOPE();
  if (canRedo()) {
    p->updateToNextVersion();
    Q_EMIT currentVersionChanged(currentVersion(), highestVersion());
  }
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
