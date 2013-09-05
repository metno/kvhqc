
#include "EditAccess.hh"
#include "Helpers.hh"

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

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
        if (eq_Sensor()(dst.sensor, sensor) and limits.contains(dst.time))
            times.insert(dst.time);
    }

    return times;
}

ObsDataPtr EditAccess::find(const SensorTime& st)
{
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
    for(Data_t::iterator it = mData.begin(); it != mData.end(); ++it) {
        EditDataPtr ebs = it->second;
        if (ebs and (ebs->modified() or ebs->modifiedTasks())) {
            updates.push_back(ObsUpdate(ebs, ebs->corrected(), ebs->controlinfo(), ebs->allTasks()));
            LOG4SCOPE_DEBUG(DBGO1(ebs));
            ebs->reset();
        }
    }
    return mBackend->update(updates);
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
template< typename T, class E>
bool popUpdates(std::vector< std::pair<int, T> >& vNew, int currentUpdate)
{
    if (vNew.empty())
        return false;
    const T& old = vNew.back().second;
    while (not vNew.empty() and vNew.back().first >= currentUpdate)
        vNew.pop_back();
    return (vNew.empty() or not E()(vNew.back().second, old));
}

template< typename T>
bool popUpdates(std::vector< std::pair<int, T> >& vNew, int currentUpdate)
{
    return popUpdates< T, std::equal_to<T> >(vNew, currentUpdate);
}

struct PopUpdate {
    EditDataPtr ebs;
    bool destroyed;
    int dU, dT;
    PopUpdate(EditDataPtr e, bool d, int du, int dt)
        : ebs(e), destroyed(d), dU(du), dT(dt) { }
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
    std::list<PopUpdate> send;
    for(Data_t::iterator it = mData.begin(); it != mData.end();) {
        EditDataPtr ebs = it->second;
        if (not ebs) {
            ++it;
            continue;
        }

        const int wasUpdated = ebs->modified()?1:0, hadTasks = ebs->hasTasks()?1:0;
        bool changed = popUpdates<float, Helpers::float_eq>(ebs->mCorrected, mUpdateCount);
        changed |= popUpdates(ebs->mControlinfo, mUpdateCount);
        changed |= popUpdates(ebs->mTasks, mUpdateCount);
        const bool destroyed = (ebs->created() and ebs->mCorrected.empty() and ebs->mControlinfo.empty() and not ebs->hasTasks());
        const int isUpdated = (not destroyed and ebs->modified())?1:0, hasTasks = ebs->hasTasks()?1:0;

        if (changed or destroyed)
            send.push_back(PopUpdate(ebs, destroyed, isUpdated - wasUpdated, hasTasks - hadTasks));
        
        Data_t::iterator eit = it++;
        if (destroyed)
            mData.erase(eit);
    }
    if (mUpdateCount>0)
        mUpdateCount -= 1;
    if (send.empty())
        return false;
    BOOST_FOREACH(const PopUpdate& p, send)
        sendObsDataChanged(p.destroyed ? DESTROYED : MODIFIED, p.ebs, p.dU, p.dT);
    return true;
}

void EditAccess::sendObsDataChanged(ObsDataChange what, ObsDataPtr obs, int dUpdated, int dTasks)
{
    mUpdated += dUpdated;
    mTasks += dTasks;
    obsDataChanged(what, obs);
}

EditDataEditorPtr EditAccess::editor(EditDataPtr obs)
{
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
    if (backendChanged) {
        const int isModified = ebs->modified()?1:0, hasTasks = ebs->hasTasks()?1:0;
        sendObsDataChanged(EditAccess::MODIFIED, ebs, isModified - wasModified, hasTasks - hadTasks);
        backendDataChanged(what, ebs);
    }
}
