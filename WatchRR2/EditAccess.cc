
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
    , mUpdateCount(0)
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
        mUpdateCount = mUpdated = mTasks = 0;
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
    mUpdateCount = mUpdated = mTasks = 0;
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

void EditAccess::pushUpdate()
{
    LOG_SCOPE("EditAccess");
    mUpdateCount += 1;
}

bool EditAccess::popUpdate()
{
    LOG_SCOPE("EditAccess");
    if (mUpdateCount>0)
        mUpdateCount -= 1;

    std::list<PopUpdate> send;
    for(Data_t::iterator it = mData.begin(); it != mData.end(); ++it) {
        EditDataPtr ebs = it->second;
        if (not ebs)
            continue;

        const int wasUpdated = ebs->modified()?1:0, hadTasks = ebs->hasTasks()?1:0;
        bool changed = ebs->mCorrected.setVersion(mUpdateCount, false);
        changed |= ebs->mControlinfo.setVersion(mUpdateCount, false);
        changed |= ebs->mTasks.setVersion(mUpdateCount, false);

        if (changed) {
            const int isUpdated = (ebs->modified())?1:0, hasTasks = ebs->hasTasks()?1:0;
            send.push_back(PopUpdate(ebs, isUpdated - wasUpdated, hasTasks - hadTasks));
        }
    }
    if (send.empty())
        return false;
    BOOST_FOREACH(const PopUpdate& p, send)
        sendObsDataChanged(MODIFIED, p.ebs, p.dU, p.dT);
    return true;
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
    if (mUpdateCount == 0)
        pushUpdate();
    return EditDataEditorPtr(new EditDataEditor(this, obs));
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
