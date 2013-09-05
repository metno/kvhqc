
#include "EditVersionModel.hh"

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

//#define NDEBUG
#include "debug.hh"

EditVersionModel::EditVersionModel(EditAccessPtr eda)
    : mDA(eda)
{
    LOG_SCOPE("EditVersionModel");
    LOG4SCOPE_DEBUG(DBG1(mDA->currentVersion()) << DBG1(mDA->highestVersion()));
    mDA->currentVersionChanged.connect(boost::bind(&EditVersionModel::onCurrentVersionChanged, this, _1, _2));
    mDA->obsDataChanged       .connect(boost::bind(&EditVersionModel::onDataChanged,           this, _1, _2));
}

EditVersionModel::~EditVersionModel()
{
    mDA->obsDataChanged       .disconnect(boost::bind(&EditVersionModel::onDataChanged,           this, _1, _2));
    mDA->currentVersionChanged.disconnect(boost::bind(&EditVersionModel::onCurrentVersionChanged, this, _1, _2));
}

void EditVersionModel::onDataChanged(ObsAccess::ObsDataChange what, ObsDataPtr obs)
{
    LOG_SCOPE("EditVersionModel");
    dump();
}

void EditVersionModel::onCurrentVersionChanged(int current, int highest)
{
    LOG_SCOPE("EditVersionModel");
    LOG4SCOPE_DEBUG(DBG1(current) << DBG1(highest));
    dump();
}

void EditVersionModel::dump()
{
    LOG_SCOPE("EditVersionModel");
    LOG4SCOPE_DEBUG(DBG1(mDA->currentVersion()) << DBG1(mDA->highestVersion()));
    for(int v=1; v<=mDA->highestVersion(); ++v) {
        const std::vector<EditDataPtr> changes = mDA->versionChanges(v);
        LOG4SCOPE_DEBUG("changes for version " << v << ":");
        BOOST_FOREACH(EditDataPtr obs, changes)
            LOG4SCOPE_DEBUG("   " << obs->sensorTime() << " c=" << obs->corrected(v) << " f=" << obs->controlinfo(v).flagstring());
    }
}
