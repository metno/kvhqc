
#include "EditAccess.hh"
#include "Helpers.hh"

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include <boost/range/adaptor/map.hpp>

#include <stdexcept>

#define MILOGGER_CATEGORY "kvhqc.EditAccess"
#include "HqcLogging.hh"

EditAccess::EditAccess(ObsAccessPtr backend)
    : mBackend(backend)
    , mVersionTimestamps(1, timeutil::now())
    , mCurrentVersion(0)
    , mUpdated(0)
    , mTasks(0)
{
    mBackend->obsDataChanged.connect(boost::bind(&EditAccess::onBackendDataChanged, this, _1, _2));
}

EditAccess::~EditAccess()
{
    mBackend->obsDataChanged.disconnect(boost::bind(&EditAccess::onBackendDataChanged, this, _1, _2));
}

void EditAccess::addEditTimes(TimeSet& times, const Sensor& sensor, const TimeRange& limits)
{
  for (Data_t::const_iterator it = mData.lower_bound(SensorTime(sensor, limits.t0())); it != mData.end(); ++it) {
    const SensorTime& dst = it->first;
    if ((not eq_Sensor()(dst.sensor, sensor)) or (dst.time > limits.t1()))
      break;
    if (it->second)
      times.insert(dst.time);
  }
}

ObsAccess::TimeSet EditAccess::allTimes(const Sensor& sensor, const TimeRange& limits)
{
    TimeSet times = mBackend->allTimes(sensor, limits);
    addEditTimes(times, sensor, limits);
    return times;
}

ObsAccess::DataSet EditAccess::allData(const Sensor& sensor, const TimeRange& limits)
{
  METLIBS_LOG_SCOPE();
  METLIBS_LOG_DEBUG(LOGVAL(sensor) << LOGVAL(limits));
  DataSet data;
  for (Data_t::const_iterator it = mData.lower_bound(SensorTime(sensor, limits.t0())); it != mData.end(); ++it) {
    const SensorTime& dst = it->first;
    if ((not eq_Sensor()(dst.sensor, sensor)) or (dst.time > limits.t1()))
      break;
    if (it->second)
      data.insert(it->second);
  }

  std::vector<ObsDataPtr> onlyBackend;
  const DataSet dataBackend = mBackend->allData(sensor, limits);
  std::set_difference(dataBackend.begin(), dataBackend.end(), data.begin(), data.end(),
      std::back_inserter(onlyBackend), lt_ObsDataPtr());
  METLIBS_LOG_DEBUG(LOGVAL(data.size()) << LOGVAL(dataBackend.size()) << LOGVAL(onlyBackend.size()));

  BOOST_FOREACH(const ObsDataPtr& obs, onlyBackend) {
    EditDataPtr ebs = boost::make_shared<EditData>(obs);
    mData[ebs->sensorTime()] = ebs;
    data.insert(ebs);
  }

  return data;
}

ObsDataPtr EditAccess::find(const SensorTime& st)
{
    if (not st.valid()) {
        METLIBS_LOG_ERROR("invalid sensorTime: " << st);
        return ObsDataPtr();
    }

    Data_t::iterator it = mData.lower_bound(st);
    if (it == mData.end() or not eq_SensorTime()(st, it->first)) {
      assert(mData.find(st) == mData.end());
      ObsDataPtr obs = mBackend->find(st);
      EditDataPtr ebs = obs ? boost::make_shared<EditData>(obs) : EditDataPtr();
      assert(mData.find(st) == mData.end());
      mData.insert(it, std::make_pair(st, ebs));
      return ebs;
    } else {
      assert(mData.find(st) == it);
      return it->second;
    }
}

ObsDataPtr EditAccess::create(const SensorTime& st)
{
    if (not st.valid())
        METLIBS_LOG_ERROR("invalid sensorTime: " << st);

    Data_t::iterator it = mData.find(st);
    if (it != mData.end() and it->second)
        return it->second;

    EditDataPtr ebs = boost::make_shared<EditData>(mBackend->create(st));
    ebs->mCreated = true;
    mData[st] = ebs;
    sendObsDataChanged(CREATED, ebs, 1, 0);
    return ebs;
}

bool EditAccess::update(const std::vector<ObsUpdate>& updates)
{
    METLIBS_LOG_SCOPE();
    BOOST_FOREACH(const ObsUpdate& ou, updates) {
        EditDataPtr ebs = findOrCreateE(ou.obs->sensorTime());
        editor(ebs)->setCorrected(ou.corrected).setControlinfo(ou.controlinfo).setTasks(ou.tasks);
    }
    return true;
}

bool EditAccess::sendChangesToParent(bool alsoSendTasks)
{
  METLIBS_LOG_SCOPE();
  std::vector<ObsUpdate> updates;
  std::vector<EditDataPtr> obsToReset;
  BOOST_FOREACH(EditDataPtr ebs, mData | boost::adaptors::map_values) {
    if (ebs and (ebs->modified() or (alsoSendTasks and ebs->modifiedTasks()))) {
      updates.push_back(ObsUpdate(ebs, ebs->corrected(), ebs->controlinfo(), alsoSendTasks ? ebs->allTasks() : 0));
      METLIBS_LOG_DEBUG(LOGOBS(ebs));
      obsToReset.push_back(ebs);
    }
  }

  const bool success = mBackend->update(updates);
  METLIBS_LOG_DEBUG(LOGVAL(success));
  if (success) {
    mVersionTimestamps = VersionTimestamps_t(1, timeutil::now());
    mCurrentVersion = mUpdated = mTasks = 0;
    /* emit */ currentVersionChanged(currentVersion(), highestVersion());
    BOOST_FOREACH(EditDataPtr ebs, obsToReset) {
      ebs->reset();
      METLIBS_LOG_DEBUG(LOGVAL(ebs->sensorTime()));
    }
  }
  return success;
}

void EditAccess::reset()
{
    METLIBS_LOG_SCOPE();
    if (mVersionTimestamps.size() > 1)
        mVersionTimestamps.erase(mVersionTimestamps.begin() + 1, mVersionTimestamps.end());
    mCurrentVersion = mUpdated = mTasks = 0;
    /* emit */ currentVersionChanged(currentVersion(), highestVersion());

    for(Data_t::iterator it = mData.begin(); it != mData.end();) {
        EditDataPtr ebs = it->second;
        Data_t::iterator oit = it++;
        if (not ebs)
            continue;

        const bool changed = ebs->modified() or ebs->modifiedTasks();
        ebs->reset();
        if (ebs->created()) {
            METLIBS_LOG_DEBUG(LOGEBS(ebs) << " des");
            obsDataChanged(DESTROYED, ebs);
            mData.erase(oit);
        } else if (changed) {
            METLIBS_LOG_DEBUG(LOGEBS(ebs) << " mod");
            obsDataChanged(MODIFIED, ebs);
        }
    }
}

namespace /* anonymous */ {
struct PopUpdate {
    EditDataPtr ebs;
    int dU, dT;
    PopUpdate(EditDataPtr e, int du, int dt)
        : ebs(e), dU(du), dT(dt) { }
};
} // namespace anonymous

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

void EditAccess::sendObsDataChanged(ObsDataChange what, ObsDataPtr obs, int dUpdated, int dTasks)
{
  METLIBS_LOG_SCOPE();
  METLIBS_LOG_DEBUG(LOGVAL(obs->sensorTime()) << LOGVAL(dUpdated) << LOGVAL(dTasks));
  mUpdated += dUpdated;
  mTasks += dTasks;
  obsDataChanged(what, obs);
}

EditDataEditorPtr EditAccess::editor(EditDataPtr obs)
{
    return EditDataEditorPtr(new EditDataEditor(this, obs));
}

bool EditAccess::commit(EditDataEditor* editor)
{
    if (mCurrentVersion == 0)
        throw std::logic_error("cannot commit editor for version 0");

    EditDataPtr obs = editor->mObs;
    const int wasModified = obs->modified()?1:0, hadTasks = obs->hasRequiredTasks()?1:0;

    const int u = currentVersion();
    bool changed = false;
    if (editor->mCorrected.commit()) {
        obs->mCorrected.setValue(u, editor->mCorrected.get());
        changed = true;
    }
    if (editor->mControlinfo.commit()) {
        obs->mControlinfo.setValue(u, editor->mControlinfo.get());
        changed = true;
    }
    if (editor->mTasks.commit()) {
        obs->mTasks.setValue(u, editor->mTasks.get());
        changed = true;
    }

    if (changed) {
        const int isModified = obs->modified()?1:0, hasTasks = obs->hasRequiredTasks()?1:0;
        sendObsDataChanged(EditAccess::MODIFIED, obs, isModified - wasModified, hasTasks - hadTasks);
    }
    return changed;
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
