
#include "EditAccess.hh"
#include "Helpers.hh"

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include <boost/range/adaptor/map.hpp>

#define NDEBUG
#include "w2debug.hh"

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

ObsAccess::TimeSet EditAccess::allTimes(const Sensor& sensor, const TimeRange& limits)
{
    TimeSet times = mBackend->allTimes(sensor, limits);

    BOOST_FOREACH(const Data_t::value_type& d, mData) {
        const SensorTime& dst = d.first;
        if (d.second and eq_Sensor()(dst.sensor, sensor) and limits.contains(dst.time))
            times.insert(dst.time);
    }

    return times;
}

ObsDataPtr EditAccess::find(const SensorTime& st)
{
    if (not st.valid()) {
        LOG4HQC_ERROR("EditAccess", "invalid sensorTime: " << st);
        return ObsDataPtr();
    }

    Data_t::iterator it = mData.find(st);
    if (it != mData.end())
        return it->second;

    ObsDataPtr obs = mBackend->find(st);
    EditDataPtr ebs = obs ? boost::make_shared<EditData>(obs) : EditDataPtr();
    mData[st] = ebs;
    return ebs;
}

ObsDataPtr EditAccess::create(const SensorTime& st)
{
    if (not st.valid())
        LOG4HQC_ERROR("EditAccess", "invalid sensorTime: " << st);

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
    LOG_SCOPE("EditAccess");
    BOOST_FOREACH(const ObsUpdate& ou, updates) {
        EditDataPtr ebs = findOrCreateE(ou.obs->sensorTime());
        editor(ebs)->setCorrected(ou.corrected).setControlinfo(ou.controlinfo).setTasks(ou.tasks);
    }
    return true;
}

bool EditAccess::sendChangesToParent()
{
    LOG_SCOPE("EditAccess");
    std::vector<ObsUpdate> updates;
    std::vector<EditDataPtr> obsToReset;
    BOOST_FOREACH(EditDataPtr ebs, mData | boost::adaptors::map_values) {
        if (ebs and (ebs->modified() or ebs->modifiedTasks())) {
            updates.push_back(ObsUpdate(ebs, ebs->corrected(), ebs->controlinfo(), ebs->allTasks()));
            LOG4SCOPE_DEBUG(DBGO1(ebs));
            obsToReset.push_back(ebs);
        }
    }

    const bool success = mBackend->update(updates);
    LOG4SCOPE_DEBUG(DBG1(success));
    if (success) {
        mCurrentVersion = mUpdated = mTasks = 0;
        BOOST_FOREACH(EditDataPtr ebs, obsToReset) {
            ebs->reset();
            LOG4SCOPE_DEBUG(DBG1(ebs->sensorTime()));
        }
    }
    return success;
}

void EditAccess::reset()
{
    LOG_SCOPE("EditAccess");
    mCurrentVersion = mUpdated = mTasks = 0;
    for(Data_t::iterator it = mData.begin(); it != mData.end();) {
        EditDataPtr ebs = it->second;
        Data_t::iterator oit = it++;
        if (not ebs)
            continue;

        const bool changed = ebs->modified() or ebs->modifiedTasks();
        ebs->reset();
        if (ebs->created()) {
            LOG4SCOPE_DEBUG(DBGOO1(ebs) << " des");
            obsDataChanged(DESTROYED, ebs);
            mData.erase(oit);
        } else if (changed) {
            LOG4SCOPE_DEBUG(DBGOO1(ebs) << " mod");
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
    LOG_SCOPE("EditAccess");
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
        const int wasUpdated = ebs->modified()?1:0, hadTasks = ebs->hasTasks()?1:0;
        bool changed = ebs->mCorrected.setVersion(mCurrentVersion, drop);
        changed |= ebs->mControlinfo.setVersion(mCurrentVersion, drop);
        changed |= ebs->mTasks.setVersion(mCurrentVersion, drop);

        if (changed) {
            const int isUpdated = (ebs->modified())?1:0, hasTasks = ebs->hasTasks()?1:0;
            updates.push_back(PopUpdate(ebs, isUpdated - wasUpdated, hasTasks - hadTasks));
        }
    }

    /* emit */ currentVersionChanged(currentVersion(), highestVersion());
    BOOST_FOREACH(const PopUpdate& p, updates)
        sendObsDataChanged(MODIFIED, p.ebs, p.dU, p.dT);
}

void EditAccess::undoVersion()
{
    LOG_SCOPE("EditAccess");
    if (mCurrentVersion == 0)
        return;
    mCurrentVersion -= 1;
    updateToCurrentVersion(false);
}

void EditAccess::redoVersion()
{
    LOG_SCOPE("EditAccess");
    if (mCurrentVersion >= highestVersion())
        return;
    mCurrentVersion += 1;
    updateToCurrentVersion(false);
}

std::vector<EditDataPtr> EditAccess::versionChanges(int version) const
{
    std::vector<EditDataPtr> haveVersion;
    BOOST_FOREACH(EditDataPtr ebs, mData | boost::adaptors::map_values) {
        if (ebs and ebs->hasVersion(version))
            haveVersion.push_back(ebs);
    }
    return haveVersion;
}

void EditAccess::sendObsDataChanged(ObsDataChange what, ObsDataPtr obs, int dUpdated, int dTasks)
{
    DBGE(LOG4HQC_DEBUG("EditAccess", DBG1(obs->sensorTime()) << DBG1(dUpdated) << DBG1(dTasks)));
    mUpdated += dUpdated;
    mTasks += dTasks;
    obsDataChanged(what, obs);
}

EditDataEditorPtr EditAccess::editor(EditDataPtr obs)
{
    if (mCurrentVersion == 0)
        newVersion();
    return EditDataEditorPtr(new EditDataEditor(this, obs));
}

bool EditAccess::commit(EditDataEditor* editor)
{
    EditDataPtr obs = editor->mObs;
    const int wasModified = obs->modified()?1:0, hadTasks = obs->hasTasks()?1:0;

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
        const int isModified = obs->modified()?1:0, hasTasks = obs->hasTasks()?1:0;
        sendObsDataChanged(EditAccess::MODIFIED, obs, isModified - wasModified, hasTasks - hadTasks);
    }
    return changed;
}

void EditAccess::onBackendDataChanged(ObsAccess::ObsDataChange what, ObsDataPtr obs)
{
    LOG_SCOPE("EditAccess");
    LOG4SCOPE_DEBUG(DBG1(what) << DBGO1(obs));

    EditDataPtr ebs = findE(SensorTime(obs->sensorTime()));
    if (not ebs)
        return;

    const int wasModified = ebs->modified()?1:0, hadTasks = ebs->hasTasks()?1:0;
    const bool backendChanged = ebs->updateFromBackend();
    LOG4SCOPE_DEBUG(DBG1(backendChanged));
    if (backendChanged) {
        const int isModified = ebs->modified()?1:0, hasTasks = ebs->hasTasks()?1:0;
        sendObsDataChanged(EditAccess::MODIFIED, ebs, isModified - wasModified, hasTasks - hadTasks);
        backendDataChanged(what, ebs);
    }
}
